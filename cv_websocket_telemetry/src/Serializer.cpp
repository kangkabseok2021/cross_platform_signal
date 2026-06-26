#include "Serializer.h"

namespace Serializer {

nlohmann::json toJson(const FrameResult& r) {
    nlohmann::json j;
    j["frame_id"]    = r.frame_id;
    j["timestamp_ms"] = r.timestamp_ms;
    j["contours"]    = nlohmann::json::array();
    for (const auto& b : r.contours) {
        j["contours"].push_back({
            {"x",      b.x},
            {"y",      b.y},
            {"width",  b.width},
            {"height", b.height},
            {"area",   b.area()}
        });
    }
    return j;
}

FrameResult fromJson(const nlohmann::json& j) {
    FrameResult r;
    r.frame_id     = j.at("frame_id").get<uint64_t>();
    r.timestamp_ms = j.at("timestamp_ms").get<uint64_t>();
    for (const auto& c : j.at("contours")) {
        BoundingRect b;
        b.x      = c.at("x").get<int>();
        b.y      = c.at("y").get<int>();
        b.width  = c.at("width").get<int>();
        b.height = c.at("height").get<int>();
        r.contours.push_back(b);
    }
    return r;
}

} // namespace Serializer
