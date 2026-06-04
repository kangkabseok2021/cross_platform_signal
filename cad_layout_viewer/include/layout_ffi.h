#pragma once
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    double*  x_coords;
    double*  y_coords;
    size_t   n_points;
    uint32_t layer_id;
} LayoutPolygon;

typedef struct {
    LayoutPolygon* polygons;
    size_t         n_polygons;
    double         bbox_min_x, bbox_min_y;
    double         bbox_max_x, bbox_max_y;
    int            error_code;  /* 0 = OK */
    char*          error_msg;   /* null when error_code == 0 */
} ParseResult;

ParseResult* parse_layout(const char* path);
void         free_parse_result(ParseResult* ptr);

#ifdef __cplusplus
}
#endif
