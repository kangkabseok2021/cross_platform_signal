#include "gst_pipeline.h"
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappsrc.h>
#include <cstring>
#include <format>
#include <stdexcept>
#include <string>
#include <vector>

struct SampleGuard {
    GstSample* s;
    ~SampleGuard() { if (s) gst_sample_unref(s); }
};

struct MapGuard {
    GstBuffer* buf; GstMapInfo map{}; bool ok{false};
    MapGuard(GstBuffer* b, GstMapFlags f) : buf(b) { ok = gst_buffer_map(b, &map, f); }
    ~MapGuard() { if (ok) gst_buffer_unmap(buf, &map); }
};

struct GstVideoPipeline::Impl {
    Config cfg;
    GstElement* src_pipeline{nullptr};
    GstElement* sink_pipeline{nullptr};
    GstElement* appsink{nullptr};
    GstElement* appsrc{nullptr};
    explicit Impl(const Config& c) : cfg(c) {}
    ~Impl() {
        if (appsink)       gst_object_unref(appsink);
        if (appsrc)        gst_object_unref(appsrc);
        if (src_pipeline)  { gst_element_set_state(src_pipeline,  GST_STATE_NULL); gst_object_unref(src_pipeline);  }
        if (sink_pipeline) { gst_element_set_state(sink_pipeline, GST_STATE_NULL); gst_object_unref(sink_pipeline); }
    }
};

GstVideoPipeline::GstVideoPipeline(const Config& cfg) : impl_(std::make_unique<Impl>(cfg)) { gst_init(nullptr, nullptr); }
GstVideoPipeline::~GstVideoPipeline() = default;

int GstVideoPipeline::run(FrameCallback cb) {
    const auto& c = impl_->cfg;
    GError* err = nullptr;

    // Source pipeline
    std::string src_desc;
    if (c.input_path.empty()) {
        int nb = (c.num_frames > 0) ? c.num_frames : 30;
        src_desc = std::format(
            "videotestsrc num-buffers={} pattern=smpte ! "
            "video/x-raw,width={},height={},framerate={}/1 ! "
            "videoconvert ! video/x-raw,format=I420 ! "
            "appsink name=sink emit-signals=false max-buffers=2 drop=true",
            nb, c.width, c.height, c.fps);
    } else {
        src_desc = std::format(
            "filesrc location=\"{}\" ! decodebin ! "
            "videoconvert ! video/x-raw,format=I420 ! "
            "appsink name=sink emit-signals=false max-buffers=2 drop=true",
            c.input_path);
    }
    impl_->src_pipeline = gst_parse_launch(src_desc.c_str(), &err);
    if (err) { std::string m=err->message; g_error_free(err); throw std::runtime_error("src: "+m); }
    impl_->appsink = gst_bin_get_by_name(GST_BIN(impl_->src_pipeline), "sink");

    // Sink pipeline
    bool write_file = !c.output_path.empty() && c.output_path != "/dev/null";
    std::string sink_caps = std::format(
        "video/x-raw,format=I420,width={},height={},framerate={}/1", c.width, c.height, c.fps);
    std::string sink_desc;
    if (write_file) {
        sink_desc = std::format(
            "appsrc name=src format=time is-live=true caps=\"{}\" ! "
            "videoconvert ! x264enc tune=zerolatency bitrate=4000 ! mp4mux ! filesink location=\"{}\"",
            sink_caps, c.output_path);
    } else {
        sink_desc = std::format(
            "appsrc name=src format=time is-live=true caps=\"{}\" ! fakesink", sink_caps);
    }
    impl_->sink_pipeline = gst_parse_launch(sink_desc.c_str(), &err);
    if (err) { std::string m=err->message; g_error_free(err); throw std::runtime_error("sink: "+m); }
    impl_->appsrc = gst_bin_get_by_name(GST_BIN(impl_->sink_pipeline), "src");

    gst_element_set_state(impl_->sink_pipeline, GST_STATE_PLAYING);
    gst_element_set_state(impl_->src_pipeline,  GST_STATE_PLAYING);

    const int luma = c.width * c.height;
    const int i420 = luma + luma/2;
    std::vector<uint8_t> luma_out(static_cast<size_t>(luma), 0);
    std::vector<uint8_t> frame_out(static_cast<size_t>(i420), 128);

    int n = 0;
    while (c.num_frames < 0 || n < c.num_frames) {
        GstSample* raw = gst_app_sink_pull_sample(GST_APP_SINK(impl_->appsink));
        if (!raw) break;
        SampleGuard sg{raw};

        GstBuffer* buf = gst_sample_get_buffer(raw);
        MapGuard mg(buf, GST_MAP_READ);
        if (!mg.ok) break;

        FrameInfo info{c.width, c.height,
            static_cast<double>(GST_BUFFER_PTS(buf)) / GST_SECOND, n};
        cb(mg.map.data, luma_out.data(), info);

        std::memcpy(frame_out.data(), luma_out.data(), static_cast<size_t>(luma));
        std::memset(frame_out.data() + luma, 128, static_cast<size_t>(luma/2));

        if (impl_->appsrc) {
            GstBuffer* ob = gst_buffer_new_allocate(nullptr, static_cast<gsize>(i420), nullptr);
            MapGuard wm(ob, GST_MAP_WRITE);
            if (wm.ok) std::memcpy(wm.map.data, frame_out.data(), static_cast<size_t>(i420));
            GST_BUFFER_PTS(ob)      = static_cast<GstClockTime>(n) * GST_SECOND / c.fps;
            GST_BUFFER_DURATION(ob) = GST_SECOND / c.fps;
            gst_app_src_push_buffer(GST_APP_SRC(impl_->appsrc), ob);
        }
        ++n;
    }

    if (impl_->appsrc) gst_app_src_end_of_stream(GST_APP_SRC(impl_->appsrc));
    if (impl_->sink_pipeline) {
        GstBus* bus = gst_element_get_bus(impl_->sink_pipeline);
        gst_bus_timed_pop_filtered(bus, 3 * GST_SECOND,
            static_cast<GstMessageType>(GST_MESSAGE_EOS | GST_MESSAGE_ERROR));
        gst_object_unref(bus);
    }
    gst_element_set_state(impl_->src_pipeline,  GST_STATE_NULL);
    gst_element_set_state(impl_->sink_pipeline, GST_STATE_NULL);
    return n;
}
