#ifndef MOCK_ENGINE_H
#define MOCK_ENGINE_H

#include "inference/IInferenceEngine.h"

class MockEngine : public IInferenceEngine {
private:
    std::vector<BoundingBox> next_result_;
public:
    void set_next_result(const std::vector<BoundingBox>& result) {
        next_result_ = result;
    }

    std::vector<BoundingBox> run(int /*frame_id*/) override {
        std::vector<BoundingBox> temp = next_result_;
        next_result_.clear();
        return temp;
    }
};

#endif // MOCK_ENGINE_H
