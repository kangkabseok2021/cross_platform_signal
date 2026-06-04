#include "gst_pipeline.h"
#include "sobel_processor.h"
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <format>
#include <iostream>
#include <string>
#include <string_view>

// ── CLI argument parsing ──────────────────────────────────────────────────────

struct Args {
    std::string input;
    std::string output    = "output.mp4";
    int         width     = 1280;
    int         height    = 720;
    int         fps       = 25;
    int         frames    = -1;
    bool        stub_gpu  = false;
    bool        benchmark = false;
    bool        json_status = false;
    bool        synthetic = false;
};

static Args parse_args(int argc, char** argv)
{
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string_view k = argv[i];
        auto next = [&]() -> std::string_view {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << k << '\n';
                std::exit(2);
            }
            return argv[++i];
        };
        if      (k == "--input")       a.input      = next();
        else if (k == "--output")      a.output     = next();
        else if (k == "--width")       a.width      = std::stoi(std::string(next()));
        else if (k == "--height")      a.height     = std::stoi(std::string(next()));
        else if (k == "--fps")         a.fps        = std::stoi(std::string(next()));
        else if (k == "--frames")      a.frames     = std::stoi(std::string(next()));
        else if (k == "--stub-gpu")    a.stub_gpu   = true;
        else if (k == "--benchmark")   a.benchmark  = true;
        else if (k == "--json-status") a.json_status = true;
        else if (k == "--synthetic")   { a.synthetic = true; a.input.clear(); }
        else if (k == "--version")     { std::cout << "endoscopic_pipeline 1.0.0\n"; std::exit(0); }
        else if (k == "--help") {
            std::cout <<
                "Usage: endoscopic_pipeline [options]\n"
                "  --input FILE        Input video (omit for synthetic pattern)\n"
                "  --output FILE       Output MP4 (default: output.mp4)\n"
                "  --width INT         Frame width  (default: 1280)\n"
                "  --height INT        Frame height (default: 720)\n"
                "  --fps INT           Frames per second (default: 25)\n"
                "  --frames INT        Number of frames to process (-1=all)\n"
                "  --stub-gpu          Force CPU Sobel even if CUDA compiled in\n"
                "  --benchmark         Write frames to /dev/null, print per-frame JSON\n"
                "  --json-status       Emit JSON status lines to stdout\n"
                "  --synthetic         Use videotestsrc (overrides --input)\n"
                "  --version           Print version and exit\n";
            std::exit(0);
        }
        else {
            std::cerr << "Unknown flag: " << k << '\n';
            std::exit(2);
        }
    }
    return a;
}

// ── main ──────────────────────────────────────────────────────────────────────

int main(int argc, char** argv)
{
    Args a = parse_args(argc, argv);

    // Fast-fail for non-existent input files — avoids GStreamer hanging during probe
    if (!a.input.empty() && !a.synthetic &&
        !std::filesystem::exists(a.input)) {
        std::cerr << std::format("Error: input file not found: {}\n", a.input);
        return 1;
    }

    auto processor = make_sobel_processor(a.stub_gpu);

    GstVideoPipeline::Config cfg;
    cfg.input_path  = a.input;
    cfg.output_path = a.benchmark ? "/dev/null" : a.output;
    cfg.width       = a.width;
    cfg.height      = a.height;
    cfg.fps         = a.fps;
    cfg.num_frames  = a.frames > 0 ? a.frames : (a.benchmark ? 60 : -1);

    if (a.json_status) {
        std::cout << std::format(
            "{{\"status\":\"ready\",\"version\":\"1.0.0\",\"processor\":\"{}\"}}\n",
            processor->name());
        std::cout.flush();
    }

    long long frame_idx = 0;
    auto t_start = std::chrono::steady_clock::now();

    try {
        GstVideoPipeline pipeline(cfg);
        pipeline.run([&](const uint8_t* luma_in, uint8_t* luma_out,
                          const FrameInfo& info)
        {
            auto t0 = std::chrono::steady_clock::now();
            processor->process(luma_in, luma_out, info.width, info.height);
            auto t1 = std::chrono::steady_clock::now();

            double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

            if (a.json_status || a.benchmark) {
                std::cout << std::format(
                    "{{\"status\":\"frame\",\"n\":{},\"ms\":{:.2f}}}\n",
                    frame_idx, ms);
                std::cout.flush();
            }
            ++frame_idx;
        });
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        return 1;
    }

    auto t_end = std::chrono::steady_clock::now();
    double total_s = std::chrono::duration<double>(t_end - t_start).count();
    double mean_fps = (total_s > 0) ? (static_cast<double>(frame_idx) / total_s) : 0.0;

    if (a.json_status || a.benchmark) {
        std::cout << std::format(
            "{{\"status\":\"done\",\"total_frames\":{},\"mean_fps\":{:.1f},\"dropped\":0}}\n",
            frame_idx, mean_fps);
        std::cout.flush();
    }
    return 0;
}
