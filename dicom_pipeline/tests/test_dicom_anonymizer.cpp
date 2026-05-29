#include <gtest/gtest.h>
#include "DicomAnonymizer.h"
#include "DicomReader.h"
#include <cstdio>

class DicomAnonymizerTest : public ::testing::Test {
protected:
    std::string fixture_path = std::string(TEST_FIXTURE_DIR) + "/sample_64x64.dcm";
    std::string out_path = std::string(TEST_FIXTURE_DIR) + "/sample_anonymized.dcm";

    void TearDown() override {
        std::remove(out_path.c_str());
    }
};

TEST_F(DicomAnonymizerTest, Sha256Hex_CorrectLengthAndDeterminism) {
    std::string text = "Hello World";
    std::string hash1 = DicomAnonymizer::sha256_hex(text);
    std::string hash2 = DicomAnonymizer::sha256_hex(text);

    EXPECT_EQ(hash1.size(), 64);
    EXPECT_EQ(hash1, hash2);
    // SHA-256 for "Hello World" is a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e
    EXPECT_EQ(hash1, "a591a6d40bf420404a011733cfb7b190d62c65bf0bcda32b57b277d9ad9f146e");
}

TEST_F(DicomAnonymizerTest, Anonymize_PatientNameIsHashed) {
    DicomAnonymizer anonymizer;
    AnonymizationResult res = anonymizer.anonymize(fixture_path, out_path);
    ASSERT_TRUE(res.success) << res.error;

    DicomReader reader;
    DicomFile anonymized = reader.read(out_path);

    std::string expected_hash = DicomAnonymizer::sha256_hex("Test^Patient");
    ASSERT_EQ(anonymized.tags.count("PatientName"), 1);
    EXPECT_EQ(anonymized.tags.at("PatientName"), expected_hash);
}

TEST_F(DicomAnonymizerTest, Anonymize_PixelDataUnchanged) {
    DicomReader reader;
    DicomFile original = reader.read(fixture_path);

    DicomAnonymizer anonymizer;
    AnonymizationResult res = anonymizer.anonymize(fixture_path, out_path);
    ASSERT_TRUE(res.success);

    DicomFile anonymized = reader.read(out_path);

    EXPECT_EQ(anonymized.pixel_data.size(), original.pixel_data.size());
    EXPECT_EQ(anonymized.pixel_data, original.pixel_data);
}

TEST_F(DicomAnonymizerTest, Anonymize_NonSensitiveTagPreserved) {
    DicomReader reader;
    DicomFile original = reader.read(fixture_path);

    DicomAnonymizer anonymizer;
    AnonymizationResult res = anonymizer.anonymize(fixture_path, out_path);
    ASSERT_TRUE(res.success);

    DicomFile anonymized = reader.read(out_path);

    ASSERT_EQ(anonymized.tags.count("Modality"), 1);
    EXPECT_EQ(anonymized.tags.at("Modality"), original.tags.at("Modality"));
}

TEST_F(DicomAnonymizerTest, Anonymize_ScrubbedTagCount_Correct) {
    DicomAnonymizer anonymizer;
    AnonymizationResult res = anonymizer.anonymize(fixture_path, out_path);
    ASSERT_TRUE(res.success);

    // Default sensitive tags are 6: PatientName, PatientID, PatientBirthDate, PatientSex, InstitutionName, ReferringPhysicianName
    // All of these should be present in our fixture and therefore scrubbed
    EXPECT_EQ(res.scrubbed_tags.size(), 6);
}
