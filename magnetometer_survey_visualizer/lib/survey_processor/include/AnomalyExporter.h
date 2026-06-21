#pragma once
#include "Types.h"
#include <string>
#include <vector>
struct AnomalyExporter {
    static bool exportCsv(const std::vector<AnomalyCandidate>& candidates,
                          const std::string& path);
    static bool exportGeoJson(const std::vector<AnomalyCandidate>& candidates,
                              double origin_lat, double origin_lon,
                              double cell_m, const std::string& path);
};
