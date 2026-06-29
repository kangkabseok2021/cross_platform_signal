import sys, os, time, math
sys.path.insert(0, os.path.dirname(__file__))
import mesh_engine

def test_module_imports():
    assert hasattr(mesh_engine, 'mesh_2d')
    assert hasattr(mesh_engine, 'smooth_2d')
    assert hasattr(mesh_engine, 'quality_report_2d')

def test_mesh_2d_non_empty():
    pts = [[0,0],[1,0],[1,1],[0,1]]
    r = mesh_engine.mesh_2d(pts, 0.5)
    assert len(r['triangles']) >= 2

def test_no_inverted_elements():
    n = 12
    pts = [[math.cos(2*math.pi*i/n), math.sin(2*math.pi*i/n)] for i in range(n)]
    r = mesh_engine.mesh_2d(pts, 0.4)
    nodes = r['nodes']
    for t in r['triangles']:
        ax, ay = nodes[t[0]]; bx, by = nodes[t[1]]; cx, cy = nodes[t[2]]
        area = 0.5 * ((bx-ax)*(cy-ay) - (cx-ax)*(by-ay))
        assert area > 0, f"Inverted triangle: {t}"

def test_smoothing_does_not_worsen_quality():
    pts = [[0,0],[3,0],[3,3],[0,3]]
    r = mesh_engine.mesh_2d(pts, 0.6)
    q_before = r['quality']['mean_aspect_ratio']
    r2 = mesh_engine.smooth_2d(r['nodes'], r['triangles'], 5)
    q_after = r2['quality']['mean_aspect_ratio']
    # Smoothing must not significantly worsen aspect ratio
    assert q_after <= q_before + 0.1, f"Smoothing worsened AR: {q_before:.3f} -> {q_after:.3f}"

def test_performance_polygon():
    n = 30
    pts = [[math.cos(2*math.pi*i/n), math.sin(2*math.pi*i/n)] for i in range(n)]
    t0 = time.perf_counter()
    mesh_engine.mesh_2d(pts, 0.15)
    elapsed = time.perf_counter() - t0
    assert elapsed < 2.0, f"mesh_2d took {elapsed:.2f}s (limit 2s)"
