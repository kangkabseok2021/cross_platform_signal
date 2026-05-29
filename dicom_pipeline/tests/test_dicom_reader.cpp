#include <gtest/gtest.h>
#include "DicomReader.h"
#include <fstream>
#include <stdexcept>

class DicomReaderTest : public ::testing::Test {
protected:
    std::string fixture_path = std::string(TEST_FIXTURE_DIR) + "/sample_64x64.dcm";
};

TEST_F(DicomReaderTest, ReadValidDicom_PixelDataSize) {
    DicomReader reader;
    DicomFile file = reader.read(fixture_path);
    
    EXPECT_EQ(file.rows, 64);
    EXPECT_EQ(file.cols, 64);
    EXPECT_EQ(file.pixel_data.size(), 4096);
}

TEST_F(DicomReaderTest, ReadValidDicom_PatientNamePresent) {
    DicomReader reader;
    DicomFile file = reader.read(fixture_path);
    
    ASSERT_EQ(file.tags.count("PatientName"), 1);
    EXPECT_EQ(file.tags.at("PatientName"), "Test^Patient");
    
    ASSERT_EQ(file.tags.count("PatientID"), 1);
    EXPECT_EQ(file.tags.at("PatientID"), "PAT001");
}

TEST_F(DicomReaderTest, ReadMissingFile_Throws) {
    DicomReader reader;
    EXPECT_THROW(reader.read("nonexistent_file_ghost.dcm"), std::runtime_error);
}

TEST_F(DicomReaderTest, ReadTruncatedFile_Throws) {
    DicomReader reader;
    std::string truncated_path = std::string(TEST_FIXTURE_DIR) + "/truncated.dcm";
    
    // Copy first 10 bytes from the valid fixture to a truncated file
    std::ifstream src(fixture_path, std::ios::binary);
    std::ofstream dest(truncated_path, std::ios::binary);
    ASSERT_TRUE(src.is_open());
    ASSERT_TRUE(dest.is_open());
    
    char buffer[10];
    src.read(buffer, 10);
    dest.write(buffer, src.gcount());
    src.close();
    dest.close();
    
    EXPECT_THROW(reader.read(truncated_path), std::runtime_error);
    std::remove(truncated_path.c_str());
}
