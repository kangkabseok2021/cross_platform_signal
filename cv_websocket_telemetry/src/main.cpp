#include "ImageProcessor.h"
#include "Serializer.h"
#include "SessionManager.h"
#include "TelemetryServer.h"
#include <opencv2/videoio.hpp>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    std::string input     = "0";
    uint16_t    port      = 9001;
    double      sigma     = 1.5;
    int         canny_low = 50, canny_high = 150;

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if      (a == "--input"      && i + 1 < argc) input      = argv[++i];
        else if (a == "--port"       && i + 1 < argc) port       = static_cast<uint16_t>(std::stoi(argv[++i]));
        else if (a == "--sigma"      && i + 1 < argc) sigma      = std::stod(argv[++i]);
        else if (a == "--canny-low"  && i + 1 < argc) canny_low  = std::stoi(argv[++i]);
        else if (a == "--canny-high" && i + 1 < argc) canny_high = std::stoi(argv[++i]);
    }

    SessionManager  mgr;
    TelemetryServer server(mgr, port);
    std::thread     srv([&] { server.run(2); });

    ImageProcessor proc(sigma, canny_low, canny_high);
    cv::VideoCapture cap;
    if (input == "0") cap.open(0);
    else              cap.open(input);
    if (!cap.isOpened()) throw std::runtime_error("cannot open: " + input);

    cv::Mat  frame;
    uint64_t dropped = 0;
    while (cap.read(frame)) {
        auto result = proc.processFrame(frame);
        if (mgr.clientCount() > 0)
            mgr.broadcast(Serializer::toJson(result).dump());
        else
            ++dropped;
    }

    server.stop();
    srv.join();
    std::printf("Processed %llu frames, dropped %llu\n",
        static_cast<unsigned long long>(proc.frameCount()),
        static_cast<unsigned long long>(dropped));
    return 0;
}
