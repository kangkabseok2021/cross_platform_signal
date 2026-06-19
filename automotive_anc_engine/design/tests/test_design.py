"""Python design-layer pytest suite for the ANC simulation."""
from __future__ import annotations

import numpy as np
import pytest

from anc.fxlms import FxLMS
from anc.imc import IMCController
from anc.noise_gen import CabinNoiseGenerator
from anc.secondary_path import SecondaryPath

FS = 48_000
N = 4800  # 0.1 s


# --- Noise generator tests ---

def test_noise_gen_produces_correct_length() -> None:
    gen = CabinNoiseGenerator(fs=FS)
    ref, primary = gen.generate(N, rpm=1200.0)
    assert len(ref) == N
    assert len(primary) == N


def test_noise_gen_reference_has_tonal_peak() -> None:
    gen = CabinNoiseGenerator(fs=FS)
    ref, _ = gen.generate(N, rpm=1200.0, road_noise_level=0.0)
    mag = np.abs(np.fft.rfft(ref))
    fund_hz = 1200.0 / 60.0 * 2.0  # 2nd engine order
    peak_bin = int(round(fund_hz * N / FS))
    # The fundamental should dominate its neighbourhood
    assert mag[peak_bin] == mag[max(0, peak_bin - 5) : peak_bin + 6].max()


def test_noise_gen_road_noise_adds_power() -> None:
    gen = CabinNoiseGenerator(fs=FS)
    _, primary_quiet = gen.generate(N, rpm=1200.0, road_noise_level=0.0)
    _, primary_noisy = gen.generate(N, rpm=1200.0, road_noise_level=0.5)
    assert float(np.var(primary_noisy)) > float(np.var(primary_quiet))


def test_noise_gen_different_rpm_gives_different_spectra() -> None:
    gen = CabinNoiseGenerator(fs=FS)
    ref_low, _ = gen.generate(N, rpm=800.0, road_noise_level=0.0)
    ref_high, _ = gen.generate(N, rpm=4000.0, road_noise_level=0.0)
    assert not np.allclose(ref_low, ref_high, atol=1e-3)


# --- Secondary path tests ---

def test_secondary_path_zero_input_zero_output() -> None:
    sp = SecondaryPath.default()
    for _ in range(20):
        assert sp.apply(0.0) == pytest.approx(0.0)


def test_secondary_path_impulse_response() -> None:
    sp = SecondaryPath.default()
    y0 = sp.apply(1.0)  # h[0] = 0.0
    y1 = sp.apply(0.0)  # h[1]*1 = 0.8
    assert y0 == pytest.approx(0.0)
    assert y1 == pytest.approx(0.8, abs=1e-5)


def test_secondary_path_apply_block_consistent() -> None:
    sp1 = SecondaryPath.default()
    sp2 = SecondaryPath.default()
    x = np.random.default_rng(0).standard_normal(50).astype(np.float32)
    y_loop = np.array([sp1.apply(s) for s in x])
    y_block = sp2.apply_block(x)
    np.testing.assert_allclose(y_loop, y_block, atol=1e-6)


# --- FxLMS tests ---

def test_fxlms_zero_mu_no_adaptation() -> None:
    sp = SecondaryPath.default()
    fxlms = FxLMS(n_weights=8, mu=0.0, leakage=0.0, sec_path=sp)
    for n in range(100):
        fxlms.process(np.sin(0.1 * n), 0.5)
    assert np.all(fxlms.weights == 0.0)


def test_fxlms_leakage_bounds_weights() -> None:
    sp = SecondaryPath.default()
    fxlms = FxLMS(n_weights=16, mu=0.05, leakage=0.01, sec_path=sp)
    for n in range(5000):
        ref = np.sin(0.1 * n)
        fxlms.process(ref, ref)
    assert float(np.sum(fxlms.weights**2)) < 1e4


def test_fxlms_converges_closed_loop() -> None:
    sp = SecondaryPath.default()
    fxlms = FxLMS(n_weights=32, mu=0.01, leakage=1e-4, sec_path=sp)
    omega = 0.15
    err_init, err_final = 0.0, 0.0
    y_prev = 0.0
    for n in range(4000):
        ref = float(np.sin(omega * n))
        e = ref - y_prev
        y_prev = fxlms.process(ref, e)
        if n < 200:
            err_init += e**2
        if n > 3800:
            err_final += e**2
    assert err_final < err_init


# --- IMC tests ---

def test_imc_zero_input_zero_output() -> None:
    imc = IMCController.make_lowpass(8, gain=0.5)
    for _ in range(50):
        assert imc.process(0.0) == pytest.approx(0.0)


def test_imc_attenuates_broadband() -> None:
    imc = IMCController.make_lowpass(16, gain=0.8)
    in_power, out_power = 0.0, 0.0
    for n in range(2000):
        e = float(np.sin(0.3 * n) + np.sin(0.7 * n))
        u = imc.process(e)
        if n > 100:
            in_power += e**2
            out_power += u**2
    assert out_power < in_power


def test_imc_reset_clears_state() -> None:
    imc = IMCController.make_lowpass(8, gain=0.5)
    for n in range(50):
        imc.process(float(np.sin(0.1 * n)))
    imc.reset()
    assert imc.process(0.0) == pytest.approx(0.0)
