"""FIR secondary path model (actuator → error microphone)."""
from __future__ import annotations

import numpy as np


class SecondaryPath:
    """Causal FIR filter modelling the speaker-to-error-mic transfer function."""

    def __init__(self, coeffs: np.ndarray) -> None:
        self.coeffs = np.array(coeffs, dtype=np.float32)
        self._buf = np.zeros(len(coeffs), dtype=np.float32)

    @classmethod
    def default(cls) -> SecondaryPath:
        return cls(np.array([0.0, 0.8, 0.15, 0.05], dtype=np.float32))

    def apply(self, x: float) -> float:
        self._buf = np.roll(self._buf, 1)
        self._buf[0] = x
        return float(np.dot(self.coeffs, self._buf))

    def apply_block(self, x: np.ndarray) -> np.ndarray:
        return np.array([self.apply(s) for s in x], dtype=np.float32)

    def reset(self) -> None:
        self._buf[:] = 0.0
