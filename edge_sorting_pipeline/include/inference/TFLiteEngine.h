#ifndef TFLITE_ENGINE_H
#define TFLITE_ENGINE_H

#include "inference/IInferenceEngine.h"
#include <memory>
#include <string>

class TFLiteEngine : public IInferenceEngine {
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
public:
    TFLiteEngine();
    ~TFLiteEngine() override;

    bool load(const std::string& model_path);
    std::vector<BoundingBox> run(int frame_id) override;
};

#endif // TFLITE_ENGINE_H
