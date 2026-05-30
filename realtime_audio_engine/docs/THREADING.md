# Threading Design

## Why lock-free instead of `std::mutex`

A mutex can block indefinitely if the OS preempts the thread holding it. In an audio
callback this causes a dropout — an audible glitch. The SPSC ring buffer uses only
`std::atomic` stores and loads with acquire/release ordering, so neither the producer
nor the consumer ever blocks.

## Thread roles and priorities

| Thread | Priority | Responsibility |
|--------|----------|----------------|
| Producer (`std::jthread`) | `SCHED_FIFO` 70 | Generates 44 samples/ms of Gaussian noise |
| Consumer (`std::jthread`) | `SCHED_FIFO` 60 | Drains ring, applies IIR, writes waveform snapshot |
| Qt main thread | Normal | UI events, `Q_PROPERTY` reads, `QTimer` diagnostics poll |

`SCHED_FIFO` requires `CAP_SYS_NICE`; the call is silently ignored in CI environments.

## False-sharing prevention

`head_` and `tail_` are placed on separate 64-byte cache lines via `alignas(64)`.
Without this, a write to `head_` (producer) would invalidate the cache line containing
`tail_` (consumer), causing up to 30× throughput degradation on multi-core Intel/ARM.

## Waveform snapshot race

`waveformSnapshot()` copies `waveform_` while the consumer may be writing to it.
This is an intentional, documented data race — the display path can tolerate a
torn read of one float without any visible artefact.
