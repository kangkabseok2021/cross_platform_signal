"""Generate golden vectors for C++ parity tests. Run once; output is pasted into test_parity.cpp."""
import numpy as np

np.random.seed(0)

# Secondary path coefficients (must match make_default_secondary_path())
SP_COEFFS = [0.0, 0.8, 0.15, 0.05]
N_SP = len(SP_COEFFS)
N_W  = 8   # FxLMS taps
MU   = 0.01
N    = 32  # samples to export

omega = 0.15   # sinusoidal reference frequency

ref = np.sin(omega * np.arange(N)).astype(np.float32)

# FIR secondary path filter
sp_buf = np.zeros(N_SP, dtype=np.float32)

def apply_sp(x: float) -> float:
    global sp_buf
    sp_buf = np.roll(sp_buf, 1)
    sp_buf[0] = x
    return float(np.dot(SP_COEFFS, sp_buf))

weights = np.zeros(N_W, dtype=np.float32)
ref_buf = np.zeros(N_W, dtype=np.float32)
filt_ref_buf = np.zeros(N_W, dtype=np.float32)

outputs = []
for n in range(N):
    x = ref[n]
    ref_buf = np.roll(ref_buf, 1); ref_buf[0] = x
    fr = apply_sp(x)
    filt_ref_buf = np.roll(filt_ref_buf, 1); filt_ref_buf[0] = fr

    y = float(np.dot(weights, ref_buf))
    outputs.append(y)

    # error = ref (simulate: no cancellation yet)
    e = x
    weights += MU * filt_ref_buf * e

outputs = np.array(outputs, dtype=np.float32)

def arr_to_cpp(arr, name):
    vals = ", ".join(f"{v:.8f}f" for v in arr)
    return f"constexpr std::array<float, {len(arr)}> {name} = {{{{{vals}}}}};"

print(arr_to_cpp(ref, "golden_ref"))
print(arr_to_cpp(outputs, "golden_output"))
print(arr_to_cpp(weights.astype(np.float32), "golden_weights_final"))
print(f"// MU={MU}, N_W={N_W}, N_SP={N_SP}, omega={omega}, N={N}")
