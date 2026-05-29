#include "DicomReader.h"
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <stdexcept>

DicomFile DicomReader::read(const std::string& path) {
    DcmFileFormat fileFormat;
    OFCondition load_cond = fileFormat.loadFile(path.c_str());
    if (!load_cond.good()) {
        throw std::runtime_error("Failed to load DICOM file: " + path + " (error: " + load_cond.text() + ")");
    }

    DcmDataset* dataset = fileFormat.getDataset();
    if (!dataset) {
        throw std::runtime_error("Failed to retrieve dataset from DICOM file: " + path);
    }

    DicomFile file;
    file.path = path;

    // Retrieve Rows and Columns
    uint16_t rows_val = 0;
    uint16_t cols_val = 0;
    if (!dataset->findAndGetUint16(DCM_Rows, rows_val).good()) {
        throw std::runtime_error("Failed to find Rows tag (0028,0010)");
    }
    if (!dataset->findAndGetUint16(DCM_Columns, cols_val).good()) {
        throw std::runtime_error("Failed to find Columns tag (0028,0011)");
    }
    file.rows = rows_val;
    file.cols = cols_val;

    // Retrieve BitsAllocated to determine pixel format
    uint16_t bits_allocated = 16;
    dataset->findAndGetUint16(DCM_BitsAllocated, bits_allocated);

    if (bits_allocated == 8) {
        const uint8_t* pixel8_ptr = nullptr;
        unsigned long count8 = 0;
        OFCondition pixel8_cond = dataset->findAndGetUint8Array(DCM_PixelData, pixel8_ptr, &count8);
        if (!pixel8_cond.good()) {
            throw std::runtime_error("Failed to find 8-bit PixelData (7FE0,0010)");
        }
        if (pixel8_ptr && count8 > 0) {
            file.pixel_data.assign(pixel8_ptr, pixel8_ptr + count8);
        }
    } else {
        const uint16_t* pixel16_ptr = nullptr;
        unsigned long count16 = 0;
        OFCondition pixel16_cond = dataset->findAndGetUint16Array(DCM_PixelData, pixel16_ptr, &count16);
        if (!pixel16_cond.good()) {
            throw std::runtime_error("Failed to find 16-bit PixelData (7FE0,0010)");
        }
        if (pixel16_ptr && count16 > 0) {
            file.pixel_data.assign(pixel16_ptr, pixel16_ptr + count16);
        }
    }

    // Verify matrix integrity
    if (file.pixel_data.size() != static_cast<size_t>(file.rows * file.cols)) {
        throw std::runtime_error("DICOM pixel matrix mismatch: pixel_data size (" + 
                                 std::to_string(file.pixel_data.size()) + ") does not match rows * cols (" + 
                                 std::to_string(file.rows * file.cols) + ")");
    }

    // Build tags map
    auto extract_tag = [&](DcmTagKey tagKey, const std::string& name) {
        OFString ofStr;
        if (dataset->findAndGetOFString(tagKey, ofStr).good()) {
            file.tags[name] = ofStr.c_str();
        }
    };

    extract_tag(DCM_PatientName, "PatientName");
    extract_tag(DCM_PatientID, "PatientID");
    extract_tag(DCM_PatientBirthDate, "PatientBirthDate");
    extract_tag(DCM_PatientSex, "PatientSex");
    extract_tag(DCM_InstitutionName, "InstitutionName");
    extract_tag(DCM_ReferringPhysicianName, "ReferringPhysicianName");
    extract_tag(DCM_Modality, "Modality");
    extract_tag(DCM_StudyDate, "StudyDate");
    extract_tag(DCM_StudyTime, "StudyTime");

    return file;
}
