#include "ImageProcessor.h"
#include <gtest/gtest.h>
#include <opencv2/imgproc.hpp>

static cv::Mat solidColor(int rows, int cols, uint8_t val) {
    return cv::Mat(rows, cols, CV_8UC3, cv::Scalar(val, val, val));
}

static cv::Mat blackRectOnWhite() {
    cv::Mat m(480, 640, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::rectangle(m, {200, 200, 100, 100}, cv::Scalar(0, 0, 0), cv::FILLED);
    return m;
}

TEST(ImageProcessor, BlackFrame_ZeroContours) {
    ImageProcessor proc;
    auto r = proc.processFrame(solidColor(480, 640, 0));
    EXPECT_TRUE(r.contours.empty());
}

TEST(ImageProcessor, WhiteFrame_ZeroContours) {
    ImageProcessor proc;
    auto r = proc.processFrame(solidColor(480, 640, 255));
    EXPECT_TRUE(r.contours.empty());
}

TEST(ImageProcessor, KnownRect_ReturnsOneContour) {
    ImageProcessor proc;
    auto r = proc.processFrame(blackRectOnWhite());
    ASSERT_GE(r.contours.size(), 1u);
    EXPECT_GE(r.contours[0].area(), 8000);
}

TEST(ImageProcessor, FrameId_IncrementsPerCall) {
    ImageProcessor proc;
    proc.processFrame(solidColor(480, 640, 0));
    proc.processFrame(solidColor(480, 640, 0));
    auto r = proc.processFrame(solidColor(480, 640, 0));
    EXPECT_EQ(r.frame_id, 3u);
}

TEST(ImageProcessor, Timestamp_IsPositiveMs) {
    ImageProcessor proc;
    auto r = proc.processFrame(solidColor(480, 640, 128));
    EXPECT_GT(r.timestamp_ms, 0u);
}

TEST(ImageProcessor, MinContourArea_FiltersSmallBlobs) {
    ImageProcessor proc(1.5, 50, 150, 100);
    cv::Mat m(480, 640, CV_8UC3, cv::Scalar(255, 255, 255));
    cv::rectangle(m, {300, 300, 3, 3}, cv::Scalar(0, 0, 0), cv::FILLED);
    auto r = proc.processFrame(m);
    EXPECT_TRUE(r.contours.empty());
}

TEST(ImageProcessor, GaussianBlur_ReducesVariance) {
    cv::Mat checker(480, 640, CV_8UC3);
    for (int y = 0; y < 480; ++y)
        for (int x = 0; x < 640; ++x)
            checker.at<cv::Vec3b>(y, x) = ((x / 16 + y / 16) % 2 == 0)
                ? cv::Vec3b{255, 255, 255} : cv::Vec3b{0, 0, 0};
    cv::Mat gray, blurred;
    cv::cvtColor(checker, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, blurred, cv::Size(5, 5), 1.5);
    cv::Scalar m1, s1, m2, s2;
    cv::meanStdDev(gray, m1, s1);
    cv::meanStdDev(blurred, m2, s2);
    EXPECT_LT(s2[0], s1[0]);
}

TEST(ImageProcessor, CannyHighThreshold_FewerEdges) {
    auto frame = blackRectOnWhite();
    ImageProcessor lo(1.5, 50, 50);
    ImageProcessor hi(1.5, 50, 200);
    auto r_lo = lo.processFrame(frame);
    auto r_hi = hi.processFrame(frame);
    EXPECT_LE(r_hi.contours.size(), r_lo.contours.size());
}
