#pragma once
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

struct FrameInfo {
    int         width;
    int         height;
    double      timestamp_s;
    long long   frame_index;
};

// Encapsulates a GStreamer source + sink pipeline for luma-plane processing.
// Source:  filesrc|videotestsrc → decodebin → videoconvert → I420 appsink
// Sink:    appsrc → videoconvert → x264enc → mp4mux → filesink
class GstVideoPipeline {
public:
    struct Config {
        std::string input_path;   // empty → use synthetic videotestsrc
        std::string output_path;  // empty or "/dev/null" → no file write
        int         width{1280};
        int         height{720};
        int         fps{25};
        int         num_frames{-1}; // -1 = unlimited
    };

    using FrameCallback = std::function<void(
        const uint8_t* luma_in,
        uint8_t*       luma_out,
        const FrameInfo& info)>;

    explicit GstVideoPipeline(const Config& cfg);
    ~GstVideoPipeline();

    // Non-copyable, non-movable
    GstVideoPipeline(const GstVideoPipeline&)            = delete;
    GstVideoPipeline& operator=(const GstVideoPipeline&) = delete;

    // Start pipeline; calls cb for each frame synchronously.
    // Returns total frames processed.
    int run(FrameCallback cb);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
