#pragma once
#include "FrameResult.h"
#include <nlohmann/json.hpp>

namespace Serializer {
    nlohmann::json toJson(const FrameResult& r);
    FrameResult    fromJson(const nlohmann::json& j);
}
