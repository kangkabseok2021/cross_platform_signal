#include "sobel_processor.h"
#include <algorithm>
#include <cmath>
#include <memory>

// ── CPU 3×3 Sobel ─────────────────────────────────────────────────────────────

void CpuSobelProcessor::process(const uint8_t* in, uint8_t* out,
                                 int width, int height)
{
    // Border pixels: set to 0 (Sobel neighbourhood extends outside image)
    for (int x = 0; x < width;  ++x) {
        out[x]                        = 0;
        out[(height - 1) * width + x] = 0;
    }
    for (int y = 0; y < height; ++y) {
        out[y * width]            = 0;
        out[y * width + width - 1] = 0;
    }

    // Interior pixels
    for (int y = 1; y < height - 1; ++y) {
        for (int x = 1; x < width - 1; ++x) {
            auto px = [&](int dy, int dx) -> int {
                return static_cast<int>(in[(y + dy) * width + (x + dx)]);
            };
            int gx = -px(-1,-1) + px(-1,+1)
                     -2*px(0,-1) + 2*px(0,+1)
                     -px(+1,-1) + px(+1,+1);
            int gy = -px(-1,-1) - 2*px(-1,0) - px(-1,+1)
                     +px(+1,-1) + 2*px(+1,0) + px(+1,+1);
            int mag = std::abs(gx) + std::abs(gy);   // L1 norm
            out[y * width + x] = static_cast<uint8_t>(std::min(mag, 255));
        }
    }
}

// ── Factory ───────────────────────────────────────────────────────────────────

#ifdef ENABLE_CUDA
// Defined in cuda/cuda_sobel_processor.cu
std::unique_ptr<ISobelProcessor> make_cuda_sobel_processor();
#endif

std::unique_ptr<ISobelProcessor> make_sobel_processor(bool stub_gpu)
{
#ifdef ENABLE_CUDA
    if (!stub_gpu)
        return make_cuda_sobel_processor();
#else
    (void)stub_gpu;
#endif
    return std::make_unique<CpuSobelProcessor>();
}
