import sys, os, time, math
sys.path.insert(0, os.path.dirname(__file__))
import mesh_engine
me = mesh_engine

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

def test_SmoothingQuality():
    # Hand-crafted mesh: unit square with one interior node offset from centre.
    mesh = {
        "nodes": [[0,0],[1,0],[1,1],[0,1],[0.1,0.1]],
        "tris": [[0,1,4],[1,2,4],[2,3,4],[3,0,4]]
    }
    q_before = me.quality_report_2d(mesh)
    smoothed = me.smooth_2d(mesh, 4, 10)
    q_after = me.quality_report_2d(smoothed)
    assert q_after["mean_aspect_ratio"] <= q_before["mean_aspect_ratio"] + 0.1

def test_performance_polygon():
    n = 30
    pts = [[math.cos(2*math.pi*i/n), math.sin(2*math.pi*i/n)] for i in range(n)]
    t0 = time.perf_counter()
    mesh_engine.mesh_2d(pts, 0.15)
    elapsed = time.perf_counter() - t0
    assert elapsed < 2.0, f"mesh_2d took {elapsed:.2f}s (limit 2s)"
