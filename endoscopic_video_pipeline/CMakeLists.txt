cmake_minimum_required(VERSION 3.21)
project(EndoscopicVideoPipeline CXX)

option(ENABLE_CUDA "Enable CUDA GPU acceleration" OFF)

# Host code: C++20; CUDA device code: C++17 (CUDA 12.x max)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── GStreamer (required) ───────────────────────────────────────────────────────
find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED
    gstreamer-1.0
    gstreamer-app-1.0
)

# ── Core library (C++20, no CUDA dependency) ──────────────────────────────────
add_library(endo_core STATIC
    src/sobel_processor.cpp
    src/gst_pipeline.cpp
)
target_include_directories(endo_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${GSTREAMER_INCLUDE_DIRS}
)
target_link_libraries(endo_core PUBLIC ${GSTREAMER_LIBRARIES})
target_compile_options(endo_core PRIVATE -Wall -Wextra)
if(MSVC)
    target_compile_options(endo_core PRIVATE /W3)
endif()

# ── CUDA kernel library (only when ENABLE_CUDA=ON) ────────────────────────────
if(ENABLE_CUDA)
    enable_language(CUDA)
    set(CMAKE_CUDA_STANDARD 17)
    set(CMAKE_CUDA_STANDARD_REQUIRED ON)

    find_package(CUDAToolkit REQUIRED)

    add_library(sobel_kernels STATIC
        cuda/sobel_kernel.cu
        cuda/cuda_sobel_processor.cu
    )
    target_include_directories(sobel_kernels PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/cuda
    )
    set_property(TARGET sobel_kernels
        PROPERTY CUDA_ARCHITECTURES 75 86 89)

    target_link_libraries(endo_core PUBLIC sobel_kernels CUDA::cudart)
    target_compile_definitions(endo_core PUBLIC ENABLE_CUDA)
endif()

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(endoscopic_pipeline src/main.cpp)
target_link_libraries(endoscopic_pipeline PRIVATE endo_core)
