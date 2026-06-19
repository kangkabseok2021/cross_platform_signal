"""Synthetic automotive cabin-noise generator (MATLAB design-phase equivalent)."""
from __future__ import annotations

import numpy as np


class CabinNoiseGenerator:
    """Generates RPM-synchronous engine harmonics + broadband road noise."""

    def __init__(self, fs: float = 48_000.0, n_harmonics: int = 3) -> None:
        self.fs = fs
        self.n_harmonics = n_harmonics

    def generate(
        self,
        n_samples: int,
        rpm: float = 1200.0,
        road_noise_level: float = 0.1,
        seed: int = 0,
    ) -> tuple[np.ndarray, np.ndarray]:
        """Return (reference_signal, primary_noise) arrays of length n_samples.

        Reference signal is the tachometer-correlated input (known harmonics).
        Primary noise is reference + broadband road noise (what the error mic sees).
        """
        rng = np.random.default_rng(seed)
        t = np.arange(n_samples) / self.fs
        fund_hz = rpm / 60.0 * 2.0  # 2nd engine order (4-cylinder firing fundamental)

        ref = np.zeros(n_samples)
        for k in range(1, self.n_harmonics + 1):
            amplitude = 1.0 / k
            ref += amplitude * np.sin(2 * np.pi * k * fund_hz * t)

        # Broadband road noise: spectrally shaped white noise
        white = rng.standard_normal(n_samples)
        b = np.ones(16) / 16  # mild low-pass to shape PSD
        road = np.convolve(white, b, mode="same") * road_noise_level

        primary = ref + road
        return ref.astype(np.float32), primary.astype(np.float32)
