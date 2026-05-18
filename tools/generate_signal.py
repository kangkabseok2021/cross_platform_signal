"""
Generate synthetic sensor signals for C++ CsvFileSource and cross-language validation.

Usage:
    uv run python tools/generate_signal.py --freq 50 --snr 20 --n 1024 --out data/signal.csv
"""
import argparse
import json
import sys
from pathlib import Path

import numpy as np
from scipy.fft import rfft, rfftfreq


def generate(freq_hz: float, amplitude: float, snr_db: float,
             sample_rate: float, n: int, seed: int) -> np.ndarray:
    """A·sin(2πft) + white noise at specified SNR."""
    rng = np.random.default_rng(seed)
    t   = np.arange(n) / sample_rate
    sig = amplitude * np.sin(2 * np.pi * freq_hz * t)
    # Noise power from SNR: P_noise = P_signal / 10^(SNR/10)
    p_signal = (amplitude ** 2) / 2.0
    p_noise  = p_signal / (10 ** (snr_db / 10.0))
    noise    = rng.normal(0, np.sqrt(p_noise), n)
    return sig + noise


def scipy_reference(signal: np.ndarray, sample_rate: float) -> dict:
    """Compute reference FFT for cross-language validation in GoogleTest."""
    N    = len(signal)
    X    = rfft(signal)
    mags = np.abs(X) / N
    freqs = rfftfreq(N, 1.0 / sample_rate)
    # Skip DC bin for fundamental detection
    peak_bin = int(np.argmax(mags[1:])) + 1
    return {
        "fundamental_hz":        float(freqs[peak_bin]),
        "fundamental_magnitude": float(mags[peak_bin]),
        "bin_resolution_hz":     float(sample_rate / N),
        "n_samples":             N,
    }


def main():
    p = argparse.ArgumentParser()
    p.add_argument("--freq",   type=float, default=50.0,   help="Signal frequency Hz")
    p.add_argument("--amp",    type=float, default=1.0,    help="Amplitude")
    p.add_argument("--snr",    type=float, default=20.0,   help="SNR dB")
    p.add_argument("--sr",     type=float, default=1000.0, help="Sample rate Hz")
    p.add_argument("--n",      type=int,   default=1024,   help="Number of samples")
    p.add_argument("--seed",   type=int,   default=42)
    p.add_argument("--out",    default="data/signal.csv",  help="Output CSV path")
    p.add_argument("--ref",    default="data/reference.json", help="Reference JSON path")
    args = p.parse_args()

    signal = generate(args.freq, args.amp, args.snr, args.sr, args.n, args.seed)
    ref    = scipy_reference(signal, args.sr)

    out = Path(args.out)
    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text("\n".join(f"{v:.10f}" for v in signal) + "\n")
    Path(args.ref).write_text(json.dumps(ref, indent=2))

    print(f"Generated {args.n} samples → {out}")
    print(f"Reference fundamental: {ref['fundamental_hz']:.2f} Hz "
          f"(bin resolution: {ref['bin_resolution_hz']:.3f} Hz)")
    print(f"Reference saved → {args.ref}")


if __name__ == "__main__":
    main()
