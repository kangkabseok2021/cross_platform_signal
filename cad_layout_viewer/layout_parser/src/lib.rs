/*!
 * Memory-safe JSON layout parser exposed via a C-FFI interface.
 *
 * C++ calls `parse_layout(path)` → receives a `*mut ParseResult` whose
 * lifetime is owned by Rust.  `free_parse_result(ptr)` gives the memory back
 * to Rust via `Box::from_raw` — C++ never calls `delete`.
 */

use serde::Deserialize;
use std::ffi::CStr;
use std::io::BufReader;
use std::os::raw::{c_char, c_int};

// ── JSON schema ───────────────────────────────────────────────────────────────

#[derive(Deserialize)]
struct JsonPolygon {
    x: Vec<f64>,
    y: Vec<f64>,
}

#[derive(Deserialize)]
struct JsonLayer {
    layer_id: u32,
    name:     String,
    polygons: Vec<JsonPolygon>,
}

#[derive(Deserialize)]
struct JsonLayout {
    layers: Vec<JsonLayer>,
}

// ── FFI types (repr(C) for C++ compatibility) ─────────────────────────────────

#[repr(C)]
pub struct LayoutPolygon {
    pub x_coords: *mut f64,
    pub y_coords: *mut f64,
    pub n_points:  usize,
    pub layer_id:  u32,
}

#[repr(C)]
pub struct ParseResult {
    pub polygons:   *mut LayoutPolygon,
    pub n_polygons: usize,
    pub bbox_min_x: f64,
    pub bbox_min_y: f64,
    pub bbox_max_x: f64,
    pub bbox_max_y: f64,
    /// 0 = success, non-zero = error (message in error_msg)
    pub error_code: c_int,
    /// Null-terminated error string; null if error_code == 0.
    pub error_msg:  *mut c_char,
}

// ── Public FFI functions ──────────────────────────────────────────────────────

/// Parse a JSON chip-layout file.  Returns an owned `*mut ParseResult`.
/// Caller MUST pass the pointer to `free_parse_result` when done.
///
/// # Safety
/// `path` must be a valid, non-null, null-terminated UTF-8 string.
#[no_mangle]
pub extern "C" fn parse_layout(path: *const c_char) -> *mut ParseResult {
    let path_str = match unsafe { CStr::from_ptr(path) }.to_str() {
        Ok(s)  => s.to_owned(),
        Err(_) => return make_error("invalid UTF-8 in path"),
    };

    match parse_impl(&path_str) {
        Ok(result) => Box::into_raw(Box::new(result)),
        Err(e)     => make_error(&e.to_string()),
    }
}

/// Release all memory previously returned by `parse_layout`.
///
/// # Safety
/// `ptr` must be a valid pointer returned by `parse_layout` and must not
/// be used after this call.
#[no_mangle]
pub unsafe extern "C" fn free_parse_result(ptr: *mut ParseResult) {
    if ptr.is_null() { return; }
    let result = Box::from_raw(ptr);

    // Free error_msg if present
    if !result.error_msg.is_null() {
        let _ = std::ffi::CString::from_raw(result.error_msg);
    }

    if !result.polygons.is_null() {
        let polys = std::slice::from_raw_parts_mut(result.polygons, result.n_polygons);
        for poly in polys.iter_mut() {
            if !poly.x_coords.is_null() {
                let _ = Vec::from_raw_parts(poly.x_coords, poly.n_points, poly.n_points);
            }
            if !poly.y_coords.is_null() {
                let _ = Vec::from_raw_parts(poly.y_coords, poly.n_points, poly.n_points);
            }
        }
        let _ = Vec::from_raw_parts(result.polygons, result.n_polygons, result.n_polygons);
    }
}

// ── Implementation ────────────────────────────────────────────────────────────

fn parse_impl(path: &str) -> Result<ParseResult, Box<dyn std::error::Error>> {
    let file   = std::fs::File::open(path)?;
    let reader = BufReader::with_capacity(1 << 20, file);  // 1 MB I/O buffer
    let layout: JsonLayout = serde_json::from_reader(reader)?;

    let mut flat: Vec<LayoutPolygon> = Vec::new();
    let mut bbox = (f64::MAX, f64::MAX, f64::MIN, f64::MIN);

    for layer in &layout.layers {
        for poly in &layer.polygons {
            if poly.x.len() != poly.y.len() { continue; }
            let n = poly.x.len();

            for &x in &poly.x { bbox.0 = bbox.0.min(x); bbox.2 = bbox.2.max(x); }
            for &y in &poly.y { bbox.1 = bbox.1.min(y); bbox.3 = bbox.3.max(y); }

            let mut xs: Vec<f64> = poly.x.clone();
            let mut ys: Vec<f64> = poly.y.clone();
            let xp = xs.as_mut_ptr(); xs.shrink_to_fit(); std::mem::forget(xs);
            let yp = ys.as_mut_ptr(); ys.shrink_to_fit(); std::mem::forget(ys);

            flat.push(LayoutPolygon { x_coords: xp, y_coords: yp, n_points: n, layer_id: layer.layer_id });
        }
    }

    let n = flat.len();
    flat.shrink_to_fit();
    let polys_ptr = flat.as_mut_ptr();
    std::mem::forget(flat);

    Ok(ParseResult {
        polygons:   polys_ptr,
        n_polygons: n,
        bbox_min_x: if bbox.0 == f64::MAX { 0.0 } else { bbox.0 },
        bbox_min_y: if bbox.1 == f64::MAX { 0.0 } else { bbox.1 },
        bbox_max_x: if bbox.2 == f64::MIN { 0.0 } else { bbox.2 },
        bbox_max_y: if bbox.3 == f64::MIN { 0.0 } else { bbox.3 },
        error_code: 0,
        error_msg:  std::ptr::null_mut(),
    })
}

fn make_error(msg: &str) -> *mut ParseResult {
    let c_msg = std::ffi::CString::new(msg).unwrap_or_default();
    Box::into_raw(Box::new(ParseResult {
        polygons:   std::ptr::null_mut(),
        n_polygons: 0,
        bbox_min_x: 0.0, bbox_min_y: 0.0,
        bbox_max_x: 0.0, bbox_max_y: 0.0,
        error_code: -1,
        error_msg:  c_msg.into_raw(),
    }))
}
