"""Filtered-x LMS adaptive filter (Python / MATLAB design reference)."""
from __future__ import annotations

import numpy as np

from .secondary_path import SecondaryPath


class FxLMS:
    """FxLMS adaptive filter with leakage.

    Cancels reference-correlated tonal components (engine harmonics).
    """

    def __init__(
        self,
        n_weights: int,
        mu: float,
        leakage: float,
        sec_path: SecondaryPath,
    ) -> None:
        self.n_weights = n_weights
        self.mu = mu
        self.leakage = leakage
        self.sec_path = sec_path
        self.weights = np.zeros(n_weights, dtype=np.float32)
        self._ref_buf = np.zeros(n_weights, dtype=np.float32)
        self._fref_buf = np.zeros(n_weights, dtype=np.float32)

    def process(self, ref: float, error: float) -> float:
        """Process one sample; return anti-noise output y(n)."""
        self._ref_buf = np.roll(self._ref_buf, 1)
        self._ref_buf[0] = ref

        filt_ref = self.sec_path.apply(ref)
        self._fref_buf = np.roll(self._fref_buf, 1)
        self._fref_buf[0] = filt_ref

        y = float(np.dot(self.weights, self._ref_buf))

        # Leaky LMS update
        self.weights = (1.0 - self.leakage) * self.weights + self.mu * self._fref_buf * error
        return y

    def process_block(self, ref: np.ndarray, error: np.ndarray) -> np.ndarray:
        return np.array([self.process(r, e) for r, e in zip(ref, error)], dtype=np.float32)

    def reset(self) -> None:
        self.weights[:] = 0.0
        self._ref_buf[:] = 0.0
        self._fref_buf[:] = 0.0
        self.sec_path.reset()
