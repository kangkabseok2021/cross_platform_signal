#ifndef I_INFERENCE_ENGINE_H
#define I_INFERENCE_ENGINE_H

#include <vector>
#include "postproc/BoundingBox.h"

class IInferenceEngine {
public:
    virtual ~IInferenceEngine() = default;
    virtual std::vector<BoundingBox> run(int frame_id) = 0;
};

#endif // I_INFERENCE_ENGINE_H
