#pragma once
#include "ISignalSource.h"
#include <fstream>
#include <stdexcept>
#include <string>

// Reads pre-generated CSV (one value per line) produced by tools/generate_signal.py.
// Complexity: O(1) per sample — sequential file read.
class CsvFileSource : public ISignalSource {
public:
    CsvFileSource(const std::string& path, double sample_rate)
        : file_(path), sr_(sample_rate) {
        if (!file_) throw std::runtime_error("Cannot open CSV: " + path);
    }

    std::vector<double> read(size_t n) override {
        std::vector<double> out;
        out.reserve(n);
        std::string line;
        while (out.size() < n && std::getline(file_, line)) {
            if (!line.empty() && line[0] != '#')
                out.push_back(std::stod(line));
        }
        // Pad with zeros if file exhausted
        out.resize(n, 0.0);
        return out;
    }

    double sampleRate() const override { return sr_; }

private:
    std::ifstream file_;
    double sr_;
};
