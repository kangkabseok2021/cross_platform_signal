# Computer Vision Mathematics

## Gaussian Blur

The 2D Gaussian kernel applied before edge detection:

```
G(x,y) = (1 / (2π·σ²)) · exp(-(x² + y²) / (2σ²))
```

**Separability:** G(x,y) = G₁D(x) · G₁D(y), so OpenCV applies two 1D
convolutions — O(k) per pixel instead of O(k²).

At σ = 1.5, 5×5 kernel row weights (normalised):

| −2 | −1 | 0 | +1 | +2 |
|---|---|---|---|---|
| 0.0453 | 0.1221 | 0.2042 | 0.1221 | 0.0453 |

## Canny Edge Detection

**Step 1 — Sobel gradients (3×3 kernels):**

```
Gx = [−1  0 +1]   Gy = [−1 −2 −1]
     [−2  0 +2]        [ 0  0  0]
     [−1  0 +1]        [+1 +2 +1]
```

**Step 2 — Gradient magnitude:**

```
|∇I(x,y)| = √(Gx² + Gy²)
```

Direction θ = atan2(Gy, Gx) rounded to 0 / 45 / 90 / 135°.

**Step 3 — Non-maximum suppression:** keep only pixels that are local
maxima along their gradient direction.

**Step 4 — Double threshold hysteresis:**
- `value > high_threshold (150)` → **strong edge** (kept)
- `low_threshold (50) < value < high_threshold` → **weak edge** (kept only
  if 8-connected to a strong edge)
- `value < low_threshold` → suppressed

## Contour Area Filter

`cv::contourArea` implements the Shoelace formula:

```
A = 0.5 · |Σᵢ xᵢ·(yᵢ₊₁ − yᵢ₋₁)|
```

Contours with area < `min_contour_area` (default 100 px²) are discarded
to eliminate single-pixel Canny noise.
