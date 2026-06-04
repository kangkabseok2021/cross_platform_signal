// CudaSobelProcessor — compiled only when ENABLE_CUDA=ON.
#include "sobel_kernel.cuh"
#include "../include/sobel_processor.h"
#include <cuda_runtime.h>
#include <stdexcept>
#include <string>

class CudaSobelProcessor final : public ISobelProcessor {
public:
    CudaSobelProcessor()
    {
        cudaError_t e = cudaStreamCreate(&stream_);
        if (e != cudaSuccess)
            throw std::runtime_error(std::string("cudaStreamCreate: ") +
                                     cudaGetErrorString(e));
    }

    ~CudaSobelProcessor() override
    {
        if (d_in_)  cudaFree(d_in_);
        if (d_out_) cudaFree(d_out_);
        if (h_in_)  cudaFreeHost(h_in_);
        if (h_out_) cudaFreeHost(h_out_);
        cudaStreamDestroy(stream_);
    }

    void process(const uint8_t* in, uint8_t* out,
                 int width, int height) override
    {
        const size_t bytes = static_cast<size_t>(width * height);
        ensure_buffers(bytes);

        // Copy input to pinned host buffer, then async H→D
        memcpy(h_in_, in, bytes);
        cudaMemcpyAsync(d_in_, h_in_, bytes,
                        cudaMemcpyHostToDevice, stream_);

        // Launch kernel
        dim3 block(16, 16);
        dim3 grid((width + 15) / 16, (height + 15) / 16);
        size_t smem = 18 * 18 * sizeof(uint8_t);
        sobel_kernel<<<grid, block, smem, stream_>>>(d_in_, d_out_, width, height);

        // Async D→H, then synchronise
        cudaMemcpyAsync(h_out_, d_out_, bytes,
                        cudaMemcpyDeviceToHost, stream_);
        cudaStreamSynchronize(stream_);

        memcpy(out, h_out_, bytes);
    }

    [[nodiscard]] const char* name() const noexcept override { return "cuda"; }

private:
    void ensure_buffers(size_t bytes) {
        if (bytes == buf_size_) return;
        if (d_in_)  { cudaFree(d_in_);  d_in_  = nullptr; }
        if (d_out_) { cudaFree(d_out_); d_out_ = nullptr; }
        if (h_in_)  { cudaFreeHost(h_in_);  h_in_  = nullptr; }
        if (h_out_) { cudaFreeHost(h_out_); h_out_ = nullptr; }
        cudaMallocHost(&h_in_,  bytes);
        cudaMallocHost(&h_out_, bytes);
        cudaMalloc(&d_in_,  bytes);
        cudaMalloc(&d_out_, bytes);
        buf_size_ = bytes;
    }

    cudaStream_t stream_{};
    uint8_t* h_in_{nullptr};
    uint8_t* h_out_{nullptr};
    uint8_t* d_in_{nullptr};
    uint8_t* d_out_{nullptr};
    size_t   buf_size_{0};
};

std::unique_ptr<ISobelProcessor> make_cuda_sobel_processor()
{
    return std::make_unique<CudaSobelProcessor>();
}
