# Real-Time Audio Processing Engine

C++20 desktop application simulating a real-time audio DSP pipeline — lock-free ring buffer, first-order IIR low-pass filter, Qt6/QML UI, and SQLite preset persistence.

## Architecture

```
realtime_audio_engine/
  include/
    AudioConfig.h        — constexpr Fs, cutoff range, kRingBufferSize, computeAlpha()
    AudioRingBuffer.h    — SPSC lock-free ring; alignas(64) head_/tail_ on separate cache lines
    IirFilter.h          — y[n] = α·x[n] + (1-α)·y[n-1]; std::atomic<float> alpha for live update

  src/
    AudioEngine.h/cpp    — jthread producer (44.1 kHz Box-Muller noise) + jthread DSP consumer
    AudioEngineWrapper.h/cpp — QObject bridge: Q_PROPERTY cutoffHz/overruns/waveformBuffer,
                               100 ms QTimer diagnostics poll, Q_INVOKABLE savePreset/loadPreset
    PresetRepository.h/cpp   — QtSql/SQLite: UNIQUE name + CHECK(cutoff_hz BETWEEN 20 AND 20000),
                               WAL mode, prepared statements (no string interpolation)
    PresetModel.h/cpp        — QAbstractListModel exposing presets to QML ListView
    main.cpp                 — QGuiApplication + QQmlApplicationEngine + context property

  qml/
    RotaryKnob.qml       — Canvas 270° arc, vertical-drag sensitivity, Binding to Engine.cutoffHz
    WaveformView.qml     — 512-sample Canvas oscilloscope, 16 ms repaint Timer
    main.qml             — Dark-theme (#1a1a2e) ApplicationWindow, status bar, preset list
```

## Signal Flow

```
Producer jthread (SCHED_FIFO 70)
  std::normal_distribution → Box-Muller noise
  AudioRingBuffer::push()  44 samples / ms tick
           │
           │ std::atomic acquire/release
           ▼
Consumer jthread (SCHED_FIFO 60)
  AudioRingBuffer::pop()
  IirFilter::process()     α·x[n] + (1-α)·y[n-1]
  waveform_[pos % 512]     snapshot for display
           │
           │ 100 ms QTimer (Qt main thread)
           ▼
AudioEngineWrapper::onPollTimer()
  diagnostics() → overrunsChanged / waveformUpdated signals
           │
           ▼
QML UI
  WaveformView (16 ms Canvas repaint)
  RotaryKnob → Binding → Engine.cutoffHz → setCutoffHz()
  ListView ← PresetModel ← PresetRepository (SQLite)
```

## Filter Mathematics

**Difference equation**: `y[n] = α · x[n] + (1 − α) · y[n−1]`

**Smoothing factor**: `α = Δt / (RC + Δt)`   where `RC = 1 / (2π · fc)`, `Δt = 1 / Fs`

| fc (Hz) | α (Fs = 44 100 Hz) |
|---------|---------------------|
| 20      | ≈ 0.0028            |
| 1 000   | ≈ 0.125             |
| 10 000  | ≈ 0.588             |
| 20 000  | ≈ 0.740             |

The z-domain pole lies at `z = 1 − α`. For 0 < α < 1 the pole is strictly inside the unit circle — unconditionally stable at all cutoff frequencies. See [FILTER-MATH.md](../realtime_audio_engine/docs/FILTER-MATH.md) for the full derivation and IIR vs FIR comparison.

## Lock-Free Design

`AudioRingBuffer<float, 2048>` is a SPSC ring with `N` a compile-time power of 2 (`static_assert`). `head_` (producer) and `tail_` (consumer) sit on separate `alignas(64)` cache lines to prevent false sharing on Intel/ARM. `push` uses `memory_order_release` after the store; `pop` uses `memory_order_acquire` before the load — the acquire/release pair guarantees sample visibility without a mutex. A mutex would risk blocking the audio thread indefinitely if the OS preempted the holder, causing an audible glitch. See [THREADING.md](../realtime_audio_engine/docs/THREADING.md).

## Build

```bash
# Core library + 13 GoogleTests (no Qt required)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_AUDIO=ON
cmake --build build --target test_audio_core -j$(nproc)
ctest --test-dir build --output-on-failure -R "IirFilter|AudioRingBuffer|AudioEngine"

# Full build including Qt app + QTest (requires Qt6)
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_AUDIO=ON
cmake --build build --target audio_app test_audio_qt -j$(nproc)
QT_QPA_PLATFORM=offscreen ctest --test-dir build --output-on-failure -R "test_audio_qt"
```

## Tests

### GoogleTest — 13 tests (no Qt)

| Suite | Test | What it verifies |
|-------|------|-----------------|
| `IirFilter` | `LowCutoff_AlphaNearZero` | fc = 20 Hz → α < 0.01 |
| `IirFilter` | `HighCutoff_AlphaSignificant` | fc = 20 kHz → 0.5 < α < 1 |
| `IirFilter` | `StepResponse_SettlesAtTau` | step input reaches ≥ 63 % after τ = Fs/(2π·fc) samples |
| `IirFilter` | `OutputBoundedByInput` | 50 k noise samples — \|y\| ≤ \|x\|_max (stability) |
| `IirFilter` | `SetCutoff_ClampsToRange` | setCutoff(0) and setCutoff(∞) stay within [20, 20 000] Hz |
| `AudioRingBuffer` | `EmptyOnInit` | available() == 0, pop returns false |
| `AudioRingBuffer` | `PushPop_SingleElement` | round-trip preserves value |
| `AudioRingBuffer` | `FifoOrder` | 1 → 2 → 3 push/pop order |
| `AudioRingBuffer` | `FullReturnsFalse` | N=4, 4th push returns false |
| `AudioRingBuffer` | `Available_Correct` | available tracks push/pop count |
| `AudioEngine` | `InjectOverrun_CountedInDiagnostics` | testInjectOverrun() increments counter |
| `AudioEngine` | `SamplesProcessed_AfterRun` | after 300 ms, samples_processed > 0 |
| `AudioEngine` | `Waveform_HasNonzeroValues` | after 300 ms, at least one non-zero sample in snapshot |

### QTest — 7 tests (Qt6 required)

| Class | Test | What it verifies |
|-------|------|-----------------|
| `TestPresetRepository` | `testSaveAndLoad_RoundTrip` | INSERT + SELECT preserves name and cutoff_hz |
| `TestPresetRepository` | `testUniqueName_Rejected` | second INSERT with same name fails UNIQUE constraint |
| `TestPresetRepository` | `testCutoffBelowRange_Rejected` | cutoff_hz = 5 fails CHECK constraint |
| `TestPresetRepository` | `testRemove_DeletesRow` | DELETE removes row, loadAll returns empty |
| `TestAudioWrapper` | `testCutoff_EmitsSignal` | setCutoffHz fires cutoffHzChanged signal once |
| `TestAudioWrapper` | `testCutoff_ClampsToRange` | out-of-range values clamp to [20, 20 000] Hz |
| `TestAudioWrapper` | `testWaveform_PopulatesAfter300ms` | waveformBuffer().size() == 512 after engine runs |
| `TestAudioWrapper` | `testPreset_RoundTrip` | savePreset + loadPreset restores cutoffHz to saved value |

## CI

| Job | Platform | Tests |
|-----|----------|-------|
| `audio-test` | ubuntu-24.04, Qt6 (`qt6-base-dev qt6-declarative-dev`) | 13 GoogleTests + 7 QTests (offscreen) |
