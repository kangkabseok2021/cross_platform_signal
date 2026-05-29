#include <gtest/gtest.h>
#include "postproc/BoundingBox.h"
#include "postproc/NMS.h"

// ── compute_iou Tests ────────────────────────────────────────────────────────

TEST(IoUTest, ZeroOverlap) {
    BoundingBox a{0.0f, 0.0f, 2.0f, 2.0f, 0.9f, 0};
    BoundingBox b{3.0f, 3.0f, 2.0f, 2.0f, 0.8f, 0};
    EXPECT_FLOAT_EQ(compute_iou(a, b), 0.0f);
}

TEST(IoUTest, Identical) {
    BoundingBox a{1.0f, 1.0f, 4.0f, 4.0f, 0.9f, 0};
    BoundingBox b{1.0f, 1.0f, 4.0f, 4.0f, 0.8f, 0};
    EXPECT_FLOAT_EQ(compute_iou(a, b), 1.0f);
}

TEST(IoUTest, PartialOverlap) {
    BoundingBox a{0.0f, 0.0f, 2.0f, 2.0f, 0.9f, 0};
    BoundingBox b{1.0f, 0.0f, 2.0f, 2.0f, 0.8f, 0};
    // Intersection: x from 1 to 2 (w=1), y from 0 to 2 (h=2). Area = 2.
    // Union: Area A (4) + Area B (4) - Intersection (2) = 6.
    // IoU: 2 / 6 = 0.33333333
    EXPECT_NEAR(compute_iou(a, b), 2.0f / 6.0f, 1e-6f);
}

TEST(IoUTest, ContainedBox) {
    BoundingBox a{0.0f, 0.0f, 4.0f, 4.0f, 0.9f, 0};
    BoundingBox b{1.0f, 1.0f, 2.0f, 2.0f, 0.8f, 0};
    // Box B is entirely inside Box A.
    // Intersection = Area B = 4.
    // Union = Area A = 16.
    // IoU = 4 / 16 = 0.25. (Which is < 1.0).
    EXPECT_FLOAT_EQ(compute_iou(a, b), 0.25f);
}

// ── non_maximum_suppression Tests ────────────────────────────────────────────

TEST(NMSTest, EmptyInput) {
    std::vector<BoundingBox> input;
    auto result = non_maximum_suppression(input, 0.5f);
    EXPECT_TRUE(result.empty());
}

TEST(NMSTest, SingleBox) {
    std::vector<BoundingBox> input = {
        BoundingBox{0.0f, 0.0f, 2.0f, 2.0f, 0.95f, 0}
    };
    auto result = non_maximum_suppression(input, 0.5f);
    ASSERT_EQ(result.size(), 1);
    EXPECT_FLOAT_EQ(result[0].confidence, 0.95f);
}

TEST(NMSTest, NonOverlapping) {
    std::vector<BoundingBox> input = {
        BoundingBox{0.0f, 0.0f, 2.0f, 2.0f, 0.95f, 0},
        BoundingBox{5.0f, 5.0f, 2.0f, 2.0f, 0.85f, 0}
    };
    auto result = non_maximum_suppression(input, 0.5f);
    EXPECT_EQ(result.size(), 2);
}

TEST(NMSTest, OverlappingSuppressed) {
    std::vector<BoundingBox> input = {
        BoundingBox{0.0f, 0.0f, 2.0f, 2.0f, 0.80f, 0}, // suppressed by higher confidence overlapping box
        BoundingBox{0.5f, 0.0f, 2.0f, 2.0f, 0.95f, 0}  // kept (higher confidence)
    };
    auto result = non_maximum_suppression(input, 0.5f);
    ASSERT_EQ(result.size(), 1);
    EXPECT_FLOAT_EQ(result[0].confidence, 0.95f);
}
