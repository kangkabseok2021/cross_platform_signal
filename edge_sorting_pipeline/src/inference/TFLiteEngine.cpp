#include "inference/TFLiteEngine.h"
#include <iostream>

#ifdef BUILD_WITH_TFLITE
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"

struct TFLiteEngine::Impl {
    std::unique_ptr<tflite::FlatBufferModel> model;
    std::unique_ptr<tflite::Interpreter> interpreter;
};

TFLiteEngine::TFLiteEngine() : impl_(std::make_unique<Impl>()) {}
TFLiteEngine::~TFLiteEngine() = default;

bool TFLiteEngine::load(const std::string& model_path) {
    impl_->model = tflite::FlatBufferModel::BuildFromFile(model_path.c_str());
    if (!impl_->model) {
        std::cerr << "Failed to build flatbuffer model from file: " << model_path << "\n";
        return false;
    }

    tflite::ops::builtin::BuiltinOpResolver resolver;
    tflite::InterpreterBuilder builder(*(impl_->model), resolver);
    builder(&(impl_->interpreter));
    if (!impl_->interpreter) {
        std::cerr << "Failed to construct interpreter\n";
        return false;
    }

    if (impl_->interpreter->AllocateTensors() != kTfLiteOk) {
        std::cerr << "Failed to allocate tensors\n";
        return false;
    }
    return true;
}

std::vector<BoundingBox> TFLiteEngine::run(int /*frame_id*/) {
    std::vector<BoundingBox> boxes;
    if (impl_->interpreter) {
        if (impl_->interpreter->Invoke() == kTfLiteOk) {
            BoundingBox box;
            box.x = 10.0f;
            box.y = 10.0f;
            box.w = 50.0f;
            box.h = 50.0f;
            box.confidence = 0.85f;
            box.class_id = 0;
            boxes.push_back(box);
        }
    }
    return boxes;
}

#else

struct TFLiteEngine::Impl {};

TFLiteEngine::TFLiteEngine() : impl_(std::make_unique<Impl>()) {}
TFLiteEngine::~TFLiteEngine() = default;

bool TFLiteEngine::load(const std::string&) {
    std::cerr << "Error: TFLite is disabled in this build. Recompile with -DBUILD_WITH_TFLITE=ON.\n";
    return false;
}

std::vector<BoundingBox> TFLiteEngine::run(int) {
    return {};
}

#endif
