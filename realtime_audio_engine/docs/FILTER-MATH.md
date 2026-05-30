# IIR Filter Mathematics

## Difference equation

```
y[n] = α · x[n] + (1 − α) · y[n−1]
```

## Smoothing factor

```
α = Δt / (RC + Δt)    where RC = 1 / (2π · fc),  Δt = 1 / Fs
```

| fc (Hz) | α (Fs = 44100 Hz) |
|---------|-------------------|
| 20      | ≈ 0.0028          |
| 1 000   | ≈ 0.125           |
| 10 000  | ≈ 0.588           |
| 20 000  | ≈ 0.740           |

## Stability

The z-domain pole lies at `z = 1 − α`. For 0 < α < 1 the pole is strictly inside the
unit circle, so the filter is unconditionally stable regardless of cutoff frequency.

## Step response

The output reaches `1 − e⁻¹ ≈ 63 %` of the final value after
`τ = Fs / (2π · fc)` samples — the same time constant as the RC circuit analogy.

## IIR vs FIR comparison

| Property | First-order IIR | FIR (N taps) |
|----------|-----------------|--------------|
| Memory | O(1) | O(N) |
| Phase | Non-linear | Linear (Type I/II) |
| Stability | Always stable (1st-order) | Always stable |
| Latency | 1 sample | N/2 samples |
| Computation | 2 multiplies, 1 add | N multiplies |

For low-latency audio processing with a simple low-pass characteristic, the
first-order IIR is optimal.
