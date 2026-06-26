#include "Serializer.h"
#include <gtest/gtest.h>

TEST(Serializer, EmptyResult_ValidJson) {
    FrameResult r;
    r.frame_id = 1;
    r.timestamp_ms = 12345;
    auto j = Serializer::toJson(r);
    EXPECT_TRUE(j.contains("frame_id"));
    EXPECT_TRUE(j.contains("timestamp_ms"));
    ASSERT_TRUE(j["contours"].is_array());
    EXPECT_TRUE(j["contours"].empty());
}

TEST(Serializer, WithContours_AllFieldsPresent) {
    FrameResult r;
    r.frame_id = 2;
    r.timestamp_ms = 9999;
    r.contours.push_back({10, 20, 100, 80});
    auto j = Serializer::toJson(r);
    ASSERT_EQ(j["contours"].size(), 1u);
    auto& c = j["contours"][0];
    EXPECT_EQ(c["x"].get<int>(), 10);
    EXPECT_EQ(c["y"].get<int>(), 20);
    EXPECT_EQ(c["width"].get<int>(), 100);
    EXPECT_EQ(c["height"].get<int>(), 80);
    EXPECT_EQ(c["area"].get<int>(), 8000);
}

TEST(Serializer, RoundTrip_PreservesData) {
    FrameResult r;
    r.frame_id = 42;
    r.timestamp_ms = 100000;
    r.contours.push_back({5, 10, 50, 60});
    auto rt = Serializer::fromJson(Serializer::toJson(r));
    EXPECT_EQ(rt.frame_id, 42u);
    EXPECT_EQ(rt.timestamp_ms, 100000u);
    ASSERT_EQ(rt.contours.size(), 1u);
    EXPECT_EQ(rt.contours[0].x,      5);
    EXPECT_EQ(rt.contours[0].y,      10);
    EXPECT_EQ(rt.contours[0].width,  50);
    EXPECT_EQ(rt.contours[0].height, 60);
}

TEST(Serializer, JsonDump_DoesNotThrow) {
    FrameResult r;
    r.frame_id = 1;
    r.timestamp_ms = 1;
    EXPECT_NO_THROW({
        auto s = Serializer::toJson(r).dump();
        EXPECT_FALSE(s.empty());
    });
}
