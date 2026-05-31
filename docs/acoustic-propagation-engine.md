# Modernizing an Acoustic Propagation Engine

C refactor portfolio project mirroring a real acoustic-software engineering task: a C99 legacy module with intentional memory hazards is replaced by a C++20 RAII+Concepts+Strategy-Pattern refactor, verified against the original via GoogleTest parity tests and benchmarked with `std::async` parallel dispatch. A Qt/QML Canvas heatmap renders the live SPL field. 15 GoogleTests — C++20 core builds on every platform with no Qt dependency.

---

## Architecture

```
legacy/acoustic.c  (C99 — intentional hazards)
  float* acoustic_alloc_grid(nx, ny)     ← caller must free (hazard #1)
  static float g_air_absorption          ← mutable global (hazard #2, non-reentrant)
  int compute_spl_grid(...)              ← no bounds check on coord arrays (hazard #3)
  r=0 → log10(0) = -Inf                 ← no guard (hazard #4)

IAcousticModel (C++20 Concept + virtual base)
  ├── InverseSquareLawModel  Lp = Lw − 20·log10(r) − 11 − α·r
  └── SabineReverbModel      T60 = 0.161·V/A  →  Lp = Lw + 10·log10(4/R)

AcousticEngine  (Strategy Pattern)
  setModel(unique_ptr<IAcousticModel>)   ← runtime model swap
  computeGrid(Lw, src, x[], y[])        ← RAII std::vector, std::span inputs

GridDispatcher<AcousticModelConcept>    (C++20 template + std::async)
  computeGridAsync(model, ...)          ← hardware_concurrency() row-strips
  No mutex — read-only model + non-overlapping output sections

Qt/QML HeatmapView  (Canvas gradient, 50×50 grid)
  AcousticViewModel : QObject           ← Q_PROPERTY splGrid + source pos
  SourceMarker.qml                      ← draggable yellow dot
  ComboBox  InverseSquareLaw | SabineReverb → Strategy swap visible in UI
```

---

## Acoustic Physics

### Inverse square law (ISO 9613-1, free field over hard plane)

```
Lp = Lw − 20·log10(r) − 11 − α·r

Lw = sound power level (dBW)
r  = distance to source (m), clamped to 0.1 m
11 = hard-plane correction (dB)
α  = 0.004 dB/m at 1 kHz, 20°C, 60% RH (ISO 9613-1 Table 3)
```

| r (m) | Lp − Lw (dB) |
|---|---|
| 1 | −11.00 |
| 10 | −31.04 |
| 100 | −51.40 |

### Sabine reverberation

```
T60 = 0.161 · V / A        (room volume m³, absorption m² sabin)
R   = A / (1 − ā)          room constant
Lp  = Lw + 10·log10(4/R)
```

---

## C++20 Features Used

| Feature | Where |
|---|---|
| `concept AcousticModelConcept` | `include/AcousticModel.h` — compile-time duck typing |
| `inline constexpr` | `src/InverseSquareLawModel.h` — replaces C `static` globals |
| `std::span<const float>` | `AcousticEngine::computeGrid` — bounds-safe array view |
| `std::async(launch::async,…)` | `include/GridDispatcher.h` — lock-free row-strip parallelism |
| `[[nodiscard]]` | All `computeSPL` + `computeGrid` methods |
| `requires` constraint on template | `GridDispatcher` — `AcousticModelConcept M` |

---

## Quick Start

```bash
# Build core + 15 GoogleTests (no Qt needed)
cmake -B build -DBUILD_ACOUSTIC=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target test_acoustic_core demo_safe -j$(nproc)
ctest --test-dir build/acoustic_propagation_engine --output-on-failure -V

# Verify C99 output against NumPy oracle
./build/acoustic_propagation_engine/demo_safe > /tmp/c_out.txt
python3 acoustic_propagation_engine/scripts/validate_c.py /tmp/c_out.txt

# Run legacy leak demo (expected: Valgrind reports 3 leaks)
valgrind --leak-check=full ./build/acoustic_propagation_engine/demo_leak

# Build Qt/QML heatmap app (requires Qt 6)
cmake -B build -DBUILD_ACOUSTIC=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build --target acoustic_app -j$(nproc)
./build/acoustic_propagation_engine/AcousticEngine/acoustic_app
```

---

## Testing

**15 GoogleTests — no Qt, no network.**

| Suite | n | What it validates |
|---|---|---|
| `InverseSquareLaw` | 4 | SPL at r=1/10/100 m vs formula, r=0 guard returns finite |
| `SabineModel` | 2 | T60 = 0.161·V/A within 0.001 s, reverberant Lp finite |
| `AcousticEngine` | 4 | Grid size=nx×ny, all finite, strategy swap differs, swap-back restores |
| `LegacyCParity` | 1 | C++20 grid matches C99 grid within 1e-4 dB on 5×5 |
| `GridDispatcher` | 1 | `computeGridAsync` == serial loop within 1e-5 dB |
| `ConstexprParams` | 1 | `static_assert(kAirAbsorption_1kHz == 0.004f)` + runtime check |
| `ModelMeta` | 2 | `name()` non-empty for both models |

### Legacy hazards catalogue

`legacy/KNOWN_HAZARDS.md` documents all four C99 defects with line references, confirmed by:
- `demo_leak` + Valgrind: 3 × "definitely lost" allocations
- `demo_safe` + `validate_c.py`: physics correct despite architectural hazards
