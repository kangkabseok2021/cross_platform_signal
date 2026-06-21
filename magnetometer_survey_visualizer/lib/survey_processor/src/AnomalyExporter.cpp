#include "AnomalyExporter.h"
#include <fstream>
#include <cmath>

#ifndef M_PI
static constexpr double M_PI = 3.14159265358979323846;
#endif

bool AnomalyExporter::exportCsv(const std::vector<AnomalyCandidate>& candidates,
                                const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;
    f << "grid_x,grid_y,depth_m,peak_nT,channel\n";
    for (const auto& c : candidates)
        f << c.grid_x << ',' << c.grid_y << ','
          << c.depth_m << ',' << c.peak_nT << ',' << c.channel << '\n';
    return f.good();
}

bool AnomalyExporter::exportGeoJson(const std::vector<AnomalyCandidate>& candidates,
                                    double origin_lat, double origin_lon,
                                    double cell_m, const std::string& path) {
    std::ofstream f(path);
    if (!f.is_open()) return false;

    // Flat-earth WGS84 conversion
    constexpr double DEG_PER_M_LAT = 1.0 / 111320.0;
    const double deg_per_m_lon =
        1.0 / (111320.0 * std::cos(origin_lat * M_PI / 180.0));

    f << "{\n  \"type\": \"FeatureCollection\",\n  \"features\": [\n";
    bool first = true;
    for (const auto& c : candidates) {
        if (!first) f << ",\n";
        first = false;
        double lat = origin_lat + c.grid_y * cell_m * DEG_PER_M_LAT;
        double lon = origin_lon + c.grid_x * cell_m * deg_per_m_lon;
        f << "    {\"type\":\"Feature\","
          << "\"geometry\":{\"type\":\"Point\","
          << "\"coordinates\":[" << lon << "," << lat << "]},"
          << "\"properties\":{\"depth_m\":" << c.depth_m
          << ",\"peak_nT\":" << c.peak_nT
          << ",\"channel\":" << c.channel << "}}";
    }
    f << "\n  ]\n}\n";
    return f.good();
}
