# CT Slice Reconstruction Engine & QML Viewer

C++20 desktop application simulating the core software of a CT scanner.

## Architecture

```
ct_core/           — headless C++20 static library (no Qt dependency)
  include/
    Types.h        — Image, Sinogram (std::span row access), FilterType enum
    FilterBank.h   — buildKernel(), applyToRow(), filterSinogram()
    Reconstructor.h — reconstruct() FBP, forwardProject() for testing
  src/
    FilterBank.cpp — Ram-Lak / Shepp-Logan DFT convolution
    Reconstructor.cpp — FBP with std::execution::par_unseq

ct_viewer/         — Qt 6/QML application (BUILD_CT_VIEWER=ON)
  CtImageProvider.{h,cpp}          — QQuickImageProvider, thread-safe QMutex update
  ReconstructionController.{h,cpp} — Q_PROPERTY bindings, QtConcurrent thread pool
  qml/
    main.qml        — dark-theme ApplicationWindow (#0d1117)
    FilterControls.qml — RadioButton filter selector + cutoff Slider
```

## Algorithm

**Filtered Back Projection**: `f(x,y) = Σᵢ Pθᵢ(x·cosθᵢ + y·sinθᵢ)`

- **Ram-Lak**: `H[k] = k/n` — linear frequency ramp, maximum sharpness
- **Shepp-Logan**: `H[k] = (k/n)·sinc(k/n)` — sinc-windowed ramp, reduces ringing

Filtering applied via DFT convolution per projection row. Back-projection parallelised with `std::execution::par_unseq` (Linux + TBB) or sequential fallback (macOS).

## Build

```bash
# Headless backend + tests (no Qt required)
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target test_ct

# QML viewer (requires Qt 6 with Quick + Concurrent)
cmake -B build -DBUILD_CT_VIEWER=ON
cmake --build build --target ct_viewer
```

## Tests (6 GoogleTests)

```
FilterBank.RamLakDcIsZero                    — H[0] = 0
FilterBank.RamLakRampMonotonicallyIncreasing — H[k] ≤ H[k+1]
FilterBank.SheppLoganLessOrEqualRamLak       — sinc window damps ramp
Reconstructor.RoundTripPointSource           — peak at phantom centre after FBP
Reconstructor.ParallelMatchesSingle          — par_unseq is deterministic
Reconstructor.FilterSwitchProducesDistinctImages — Ram-Lak ≠ Shepp-Logan output
```

## CI

| Job | What |
|-----|------|
| `cpp-build-test` | Ubuntu + Windows matrix — all existing tests |
| `ct-backend-test` | Ubuntu, TBB installed, 6 CT GoogleTests via ctest |
| `clang-tidy` | `cppcoreguidelines-*` + `modernize-*` + `performance-*` on `ct_core/` |
| `python-tests` | Python tools pytest suite |
