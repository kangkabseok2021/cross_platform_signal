"""IMC (Internal Model Control) feedback controller for broadband noise."""
from __future__ import annotations

import numpy as np


class IMCController:
    """Fixed-coefficient FIR low-pass filter approximating Q(z) = S_hat^{-1}(z)*F(z)."""

    def __init__(self, coeffs: np.ndarray) -> None:
        self.coeffs = np.array(coeffs, dtype=np.float32)
        self._buf = np.zeros(len(coeffs), dtype=np.float32)

    @classmethod
    def make_lowpass(cls, n_taps: int = 8, gain: float = 0.5) -> IMCController:
        i = np.arange(n_taps)
        w = 0.5 * (1.0 - np.cos(2.0 * np.pi * i / (n_taps - 1)))  # Hann window
        c = gain * w / (n_taps / 2.0)
        return cls(c.astype(np.float32))

    def process(self, error: float) -> float:
        self._buf = np.roll(self._buf, 1)
        self._buf[0] = error
        return float(np.dot(self.coeffs, self._buf))

    def process_block(self, error: np.ndarray) -> np.ndarray:
        return np.array([self.process(e) for e in error], dtype=np.float32)

    def reset(self) -> None:
        self._buf[:] = 0.0
