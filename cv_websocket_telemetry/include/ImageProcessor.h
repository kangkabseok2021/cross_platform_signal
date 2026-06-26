#pragma once
#include "FrameResult.h"
#include <opencv2/core.hpp>
#include <cstdint>

class ImageProcessor {
public:
    explicit ImageProcessor(double gauss_sigma = 1.5,
                            int    canny_low   = 50,
                            int    canny_high  = 150,
                            int    min_area    = 100);

    FrameResult processFrame(const cv::Mat& frame);
    [[nodiscard]] uint64_t frameCount() const noexcept;

private:
    double   sigma_;
    int      canny_low_, canny_high_, min_area_;
    uint64_t frame_count_{0};
    cv::Mat  blurred_, edges_;
};
