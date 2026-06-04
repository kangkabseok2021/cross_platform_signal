#pragma once
#include <cuda_runtime.h>
#include <cstdint>

// 3×3 Sobel with 16×16 thread blocks and 18×18 shared-memory halo.
// Input / output: luma (Y) plane of an I420 frame, width×height bytes.
__global__ void sobel_kernel(
    const uint8_t* __restrict__ in,
    uint8_t*       __restrict__ out,
    int width, int height);
