#ifndef BOUNDING_BOX_H
#define BOUNDING_BOX_H

struct BoundingBox {
    float x;          // Top-left x
    float y;          // Top-left y
    float w;          // Width
    float h;          // Height
    float confidence; // Inference confidence (0.0 to 1.0)
    int class_id;     // Class identifier
};

float compute_iou(const BoundingBox& a, const BoundingBox& b);

#endif // BOUNDING_BOX_H
