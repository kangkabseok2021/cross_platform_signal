#include "ImageProcessor.h"
#include <chrono>
#include <opencv2/imgproc.hpp>

ImageProcessor::ImageProcessor(double gauss_sigma, int canny_low, int canny_high, int min_area)
    : sigma_(gauss_sigma), canny_low_(canny_low), canny_high_(canny_high), min_area_(min_area) {}

FrameResult ImageProcessor::processFrame(const cv::Mat& frame) {
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred_, cv::Size(5, 5), sigma_);
    cv::Canny(blurred_, edges_, canny_low_, canny_high_);

    std::vector<std::vector<cv::Point>> raw;
    cv::findContours(edges_, raw, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    FrameResult result;
    result.frame_id    = ++frame_count_;
    result.timestamp_ms = static_cast<uint64_t>(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());

    for (const auto& c : raw) {
        if (cv::contourArea(c) >= min_area_) {
            auto r = cv::boundingRect(c);
            result.contours.push_back({r.x, r.y, r.width, r.height});
        }
    }
    return result;
}

uint64_t ImageProcessor::frameCount() const noexcept { return frame_count_; }
