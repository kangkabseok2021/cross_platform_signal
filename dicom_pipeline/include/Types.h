#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>

struct DicomFile {
    std::string path;
    std::vector<uint16_t> pixel_data;
    std::map<std::string, std::string> tags;
    uint32_t rows{0};
    uint32_t cols{0};
};

struct AnonymizationResult {
    std::string input_path;
    std::string output_path;
    std::vector<std::string> scrubbed_tags;
    bool success{false};
    std::string error;
};

struct ProcessingStats {
    int64_t ts_ns{0};
    size_t file_size_bytes{0};
    int64_t duration_us{0};
    std::string status;
    size_t tags_scrubbed{0};
};
