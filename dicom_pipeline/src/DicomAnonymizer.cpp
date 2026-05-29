#include "DicomAnonymizer.h"
#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdatset.h>
#include <dcmtk/dcmdata/dcdeftag.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <memory>
#include <algorithm>
#include <cstring>
#include <cstdio>

static DcmTagKey get_dcm_tag_key(const std::string& name) {
    if (name == "PatientName") return DCM_PatientName;
    if (name == "PatientID") return DCM_PatientID;
    if (name == "PatientBirthDate") return DCM_PatientBirthDate;
    if (name == "PatientSex") return DCM_PatientSex;
    if (name == "InstitutionName") return DCM_InstitutionName;
    if (name == "ReferringPhysicianName") return DCM_ReferringPhysicianName;
    return DCM_UndefinedTagKey;
}

DicomAnonymizer::DicomAnonymizer()
    : sensitive_tags_({"PatientName", "PatientID", "PatientBirthDate", "PatientSex", "InstitutionName", "ReferringPhysicianName"}) {}

DicomAnonymizer::DicomAnonymizer(const std::vector<std::string>& sensitive_tags)
    : sensitive_tags_(sensitive_tags) {}

std::string DicomAnonymizer::sha256_hex(const std::string& input) {
    std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)> ctx(EVP_MD_CTX_new(), EVP_MD_CTX_free);
    if (!ctx) {
        throw std::runtime_error("Failed to create OpenSSL EVP_MD_CTX");
    }

    if (EVP_DigestInit_ex(ctx.get(), EVP_sha256(), nullptr) != 1) {
        throw std::runtime_error("Failed to initialize OpenSSL SHA-256 digest");
    }

    if (EVP_DigestUpdate(ctx.get(), input.data(), input.size()) != 1) {
        throw std::runtime_error("Failed to update OpenSSL SHA-256 digest");
    }

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int length = 0;
    if (EVP_DigestFinal_ex(ctx.get(), hash, &length) != 1) {
        throw std::runtime_error("Failed to finalize OpenSSL SHA-256 digest");
    }

    char hex[EVP_MAX_MD_SIZE * 2 + 1];
    std::memset(hex, 0, sizeof(hex));
    for (unsigned int i = 0; i < length; ++i) {
        std::snprintf(hex + 2 * i, 3, "%02x", hash[i]);
    }
    return std::string(hex, length * 2);
}

AnonymizationResult DicomAnonymizer::anonymize(const std::string& input_path, const std::string& output_path) {
    AnonymizationResult result;
    result.input_path = input_path;
    result.output_path = output_path;

    DcmFileFormat fileFormat;
    OFCondition load_cond = fileFormat.loadFile(input_path.c_str());
    if (!load_cond.good()) {
        result.success = false;
        result.error = "Failed to load input file: " + std::string(load_cond.text());
        return result;
    }

    DcmDataset* dataset = fileFormat.getDataset();
    if (!dataset) {
        result.success = false;
        result.error = "Failed to retrieve dataset from file";
        return result;
    }

    for (const auto& name : sensitive_tags_) {
        DcmTagKey tagKey = get_dcm_tag_key(name);
        if (tagKey == DCM_UndefinedTagKey) {
            continue;
        }

        OFString value;
        if (dataset->findAndGetOFString(tagKey, value).good() && !value.empty()) {
            std::string hashed_val = sha256_hex(std::string(value.c_str()));
            if (dataset->putAndInsertOFStringArray(tagKey, hashed_val.c_str()).good()) {
                result.scrubbed_tags.push_back(name);
            }
        }
    }

    OFCondition save_cond = fileFormat.saveFile(output_path.c_str());
    if (!save_cond.good()) {
        result.success = false;
        result.error = "Failed to save anonymized file: " + std::string(save_cond.text());
        return result;
    }

    result.success = true;
    return result;
}
