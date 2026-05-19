# Cross-Platform Signal Analyzer

A C++17 signal processing suite with abstract OOP interfaces, Cooley-Tukey FFT, 4th-order Butterworth IIR filtering, SQLite run logging, and an optional Qt 6 dashboard. A Python tool generates test signals and SciPy-computed reference FFT values for cross-language numerical validation. A single `CMakeLists.txt` builds on Linux and Windows without modification.

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│  tools/generate_signal.py  (Python / NumPy / SciPy)            │
│  A·sin(2πft) + Gaussian noise  →  data/signal.csv              │
│  SciPy reference FFT           →  data/reference.json          │
└────────────────────┬────────────────────────────────────────────┘
                     │ CSV
┌────────────────────▼────────────────────────────────────────────┐
│  ISignalSource  (abstract)                                      │
│  ├── SyntheticSource    A·sin(2πft) + Gaussian noise            │
│  └── CsvFileSource      sequential CSV reader, zero-pads        │
└────────────────────┬────────────────────────────────────────────┘
                     │ std::vector<double>
┌────────────────────▼────────────────────────────────────────────┐
│  ISignalFilter  (abstract)                                      │
│  ├── FftAnalyser        Cooley-Tukey radix-2 DIT, O(N log N)    │
│  ├── ButterworthFilter  4th-order IIR Direct-Form II, O(N)      │
│  └── MovingAverageFilter sliding window, O(N)                   │
└────────────────────┬────────────────────────────────────────────┘
                     │
┌────────────────────▼────────────────────────────────────────────┐
│  SignalProcessor                                                │
│  source.read(N) → filter.apply() → fft.analyse() → log         │
└──────┬─────────────────────────────────────┬────────────────────┘
       │                                     │
┌──────▼──────────────────┐     ┌────────────▼────────────────────┐
│  Qt 6 Dashboard         │     │  SqliteLogger                   │
│  Raw + filtered charts  │     │  signal_runs (bundled SQLite)   │
│  FFT spectrum view      │     │  run_id · ts · rms · latency_us │
└─────────────────────────┘     └────────────────────────────────┘
```

---

## Design Patterns

**Abstract source and filter interfaces** — `ISignalSource` and `ISignalFilter` each expose a single pure-virtual method (`read()` and `apply()`). `SignalProcessor` depends only on these interfaces; swapping a `SyntheticSource` for `CsvFileSource`, or a `ButterworthFilter` for `FftAnalyser`, requires no change to the orchestration code.

**Bundled SQLite** — `find_package(SQLite3)` was replaced with a statically compiled `sqlite3_bundled` target from `third_party/sqlite3.c`. This removes all external C++ dependencies — the project builds with only a compiler and CMake on any platform.

**Cross-language validation** — `generate_signal.py` writes a `reference.json` containing the SciPy-computed fundamental frequency and top harmonics. GoogleTest loads this file and compares the C++ FFT output against the Python reference values, catching numerical drift between platforms and compilers.

---

## Algorithms

### FFT Analyser — O(N log N)

Cooley-Tukey radix-2 DIT (decimation-in-time) in pure C++17. Input zero-padded to next power-of-2 automatically.

```
Frequency bin resolution = sample_rate / N
Output: |X[k]| / N  for k = 0 … N/2   (N/2 + 1 values, Nyquist-limited)
```

Production path: swap `FftAnalyser.cpp` for `fftw_plan_dft_r2c_1d` via the Conan-declared `fftw/3.3.10` dependency. FFTW wisdom file pre-computed at first run for deterministic startup latency.

### Butterworth Low-Pass — O(N)

4th-order Butterworth as two cascaded biquad sections in **Direct-Form II transposed** — the numerically stable canonical form that avoids intermediate-value overflow at high frequencies.

```
H(s) = 1 / [(s/ωc)⁴ + 2.613(s/ωc)³ + 3.414(s/ωc)² + 2.613(s/ωc) + 1]
```

Coefficients computed analytically at construction via bilinear transform. No lookup table — valid for any cutoff frequency and sample rate. Validated against SciPy `butter` + `sosfilt` reference values in GoogleTest.

### Moving Average — O(N)

Sliding window with running sum: O(1) per sample, internal state preserved across successive `apply()` calls.

---

## Python Signal Generator

```bash
uv sync
uv run python tools/generate_signal.py --freq 50 --snr 20 --n 1024
# → data/signal.csv       consumed by CsvFileSource
# → data/reference.json   SciPy FFT reference for GoogleTest
```

| Flag | Default | Description |
|---|---|---|
| `--freq` | 50.0 | Signal frequency (Hz) |
| `--amp` | 1.0 | Amplitude |
| `--snr` | 20.0 | SNR (dB) |
| `--sr` | 1000.0 | Sample rate (Hz) |
| `--n` | 1024 | Sample count |
| `--seed` | 42 | RNG seed for reproducibility |

---

## Qt 6 Dashboard (optional)

```bash
cmake -B build -DBUILD_GUI=ON
cmake --build build -j$(nproc)
./build/signal_dashboard
```

Two `QtCharts::QLineSeries` panels (raw signal + filtered), FFT spectrum view with magnitude vs. frequency bin, control panel for source/filter/cutoff selection. Requires Qt 6 `Widgets` + `Charts` modules.

---

## SQLite Schema

```sql
CREATE TABLE signal_runs (
    run_id         INTEGER PRIMARY KEY AUTOINCREMENT,
    ts             TEXT    NOT NULL,      -- ISO 8601 UTC
    source_type    TEXT    NOT NULL,      -- "synthetic" | "csv"
    filter_type    TEXT    NOT NULL,      -- "FFT" | "Butterworth" | "MovingAverage"
    sample_count   INTEGER NOT NULL,
    fundamental_hz REAL,                  -- dominant frequency (FFT peak bin)
    rms_raw        REAL,
    rms_filtered   REAL,
    latency_us     INTEGER               -- end-to-end processing time
);
```

---

## Testing

### C++ — GoogleTest (19 tests)

```bash
ctest --test-dir build --output-on-failure -V
```

| Suite | n | Tests |
|---|---|---|
| `FftTest` | 7 | FundamentalFrequencyDetected, MagnitudeNonZero, OutputLengthIsHalfPlusOne, FrequencyBinResolution, DCComponentAtBinZero, ZeroPaddingWorksForNonPow2Input, TwoTonesHaveTwoPeaks |
| `ButterworthTest` | 5 | PassbandSignalPassesThrough, StopbandSignalAttenuated, OutputLengthMatchesInput, NameIsButterworth, AtCutoffIs3dBDown |
| `SyntheticSourceTest` | 5 | ReadCount, SampleRate, FirstSampleNearZero, PeakAmplitude, SequentialContinuity |
| `CsvFileSourceTest` | 2 | ReadsCorrectValues, ZeroPadsOnFileExhaustion |

### Python — pytest (5 tests)

```bash
uv run pytest tools/ -v
```

CSV line count, reference JSON schema, frequency accuracy ±2 bins, float parseability, seed reproducibility.

---

## Conan 2 Integration

For production FFTW3 builds:

```bash
pip install conan
conan profile detect
conan install . --output-folder=build --build=missing
cmake -B build --preset conan-release
cmake --build build
```

`conanfile.txt` declares `fftw/3.3.10` (FFTW3 double-precision) and `gtest/1.14.0`. Replaces the Cooley-Tukey implementation with FFTW3 without changing any application code — `FftAnalyser.cpp` is the only file that differs.
