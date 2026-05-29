#include "postproc/NMS.h"
#include <algorithm>
#include <cmath>

float compute_iou(const BoundingBox& a, const BoundingBox& b) {
    float x_left = std::max(a.x, b.x);
    float y_top = std::max(a.y, b.y);
    float x_right = std::min(a.x + a.w, b.x + b.w);
    float y_bottom = std::min(a.y + a.h, b.y + b.h);

    if (x_right <= x_left || y_bottom <= y_top) {
        return 0.0f;
    }

    float intersection_area = (x_right - x_left) * (y_bottom - y_top);
    float area_a = a.w * a.h;
    float area_b = b.w * b.h;
    float union_area = area_a + area_b - intersection_area;

    if (union_area <= 0.0f) {
        return 0.0f;
    }

    return intersection_area / union_area;
}

std::vector<BoundingBox> non_maximum_suppression(std::vector<BoundingBox> boxes, float iou_threshold) {
    std::sort(boxes.begin(), boxes.end(), [](const BoundingBox& a, const BoundingBox& b) {
        return a.confidence > b.confidence;
    });

    std::vector<BoundingBox> selected;
    std::vector<bool> suppressed(boxes.size(), false);

    for (size_t i = 0; i < boxes.size(); ++i) {
        if (suppressed[i]) {
            continue;
        }

        selected.push_back(boxes[i]);

        for (size_t j = i + 1; j < boxes.size(); ++j) {
            if (suppressed[j]) {
                continue;
            }

            if (boxes[i].class_id == boxes[j].class_id) {
                float iou = compute_iou(boxes[i], boxes[j]);
                if (iou > iou_threshold) {
                    suppressed[j] = true;
                }
            }
        }
    }

    return selected;
}
