#pragma once
#include "Types.h"
#include <string>
#include <vector>

class DicomAnonymizer {
public:
    DicomAnonymizer();
    explicit DicomAnonymizer(const std::vector<std::string>& sensitive_tags);
    ~DicomAnonymizer() = default;

    // Hashes sensitive metadata fields in the input file and saves anonymized data to output path
    AnonymizationResult anonymize(const std::string& input_path, const std::string& output_path);

    // Static helper to compute deterministic SHA-256 hex string via OpenSSL EVP API
    static std::string sha256_hex(const std::string& input);

private:
    std::vector<std::string> sensitive_tags_;
};
