#pragma once
#include "Types.h"
#include <string>

class DicomReader {
public:
    DicomReader() = default;
    ~DicomReader() = default;

    // Reads a DICOM file, parses binary structure, extracts tags & pixel data, and verifies integrity
    DicomFile read(const std::string& path);
};
