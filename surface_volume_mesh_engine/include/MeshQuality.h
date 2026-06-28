#pragma once
#include "Geometry2D.h"
#include "Geometry3D.h"

struct QualityStats2D {
    double mean_aspect_ratio  = 0;
    double min_aspect_ratio   = 0;
    double mean_min_angle_deg = 0;
    double min_angle_deg      = 0;
    double frac_below_20deg   = 0;
};

struct QualityStats3D {
    double mean_edge_ratio    = 0;
    double min_edge_ratio     = 0;
    int    n_inverted         = 0;
};

double aspect_ratio_2d(const Mesh2D& m, const Triangle2D& t);
double min_angle_deg_2d(const Mesh2D& m, const Triangle2D& t);
double jacobian_det(const Mesh3D& m, const Tet& t);
double edge_ratio_3d(const Mesh3D& m, const Tet& t);

QualityStats2D quality_report_2d(const Mesh2D& mesh);
QualityStats3D quality_report_3d(const Mesh3D& mesh);
