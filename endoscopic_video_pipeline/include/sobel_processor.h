#pragma once
#include "frame_buffer.h"
#include <cstdint>
#include <memory>

// Abstract interface — runtime-selectable CPU or CUDA implementation.
class ISobelProcessor {
public:
    virtual ~ISobelProcessor() = default;

    // Process luma plane in-place: in → apply Sobel → out.
    // Both buffers must be width×height bytes (I420 Y plane).
    virtual void process(const uint8_t* in, uint8_t* out,
                         int width, int height) = 0;

    [[nodiscard]] virtual const char* name() const noexcept = 0;
};

// CPU 3×3 Sobel — always available (no CUDA required).
class CpuSobelProcessor final : public ISobelProcessor {
public:
    void process(const uint8_t* in, uint8_t* out,
                 int width, int height) override;
    [[nodiscard]] const char* name() const noexcept override { return "cpu"; }
};

// Factory: returns CudaSobelProcessor when ENABLE_CUDA=ON and !stub_gpu,
//          otherwise returns CpuSobelProcessor.
[[nodiscard]] std::unique_ptr<ISobelProcessor>
make_sobel_processor(bool stub_gpu = false);
