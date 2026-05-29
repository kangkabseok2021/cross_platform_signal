#ifndef NMS_H
#define NMS_H

#include <vector>
#include "BoundingBox.h"

std::vector<BoundingBox> non_maximum_suppression(std::vector<BoundingBox> boxes, float iou_threshold);

#endif // NMS_H
