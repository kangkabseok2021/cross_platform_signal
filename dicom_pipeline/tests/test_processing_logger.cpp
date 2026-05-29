#include <gtest/gtest.h>
#include "ProcessingLogger.h"
#include <sstream>
#include <nlohmann/json.hpp>

TEST(ProcessingLoggerTest, LogToStream_ValidNdjson) {
    std::ostringstream ss;
    ProcessingLogger logger(ss);

    ProcessingStats stats;
    stats.ts_ns = 123456789;
    stats.file_size_bytes = 1000000; // 1 MB
    stats.duration_us = 500000;      // 0.5 seconds
    stats.status = "success";
    stats.tags_scrubbed = 6;

    logger.log(stats);
    logger.flush();

    std::string line = ss.str();
    ASSERT_FALSE(line.empty());
    
    // Parse as JSON
    auto j = nlohmann::json::parse(line);
    EXPECT_EQ(j["ts_ns"], 123456789);
    EXPECT_EQ(j["file_size_bytes"], 1000000);
    EXPECT_EQ(j["duration_us"], 500000);
    EXPECT_DOUBLE_EQ(j["throughput_mb_per_s"], 2.0); // 1,000,000 bytes / 500,000 us = 2.0 MB/s
    EXPECT_EQ(j["status"], "success");
    EXPECT_EQ(j["tags_scrubbed"], 6);
}

TEST(ProcessingLoggerTest, LogMultipleRecords_FlushesOnFlush) {
    std::ostringstream ss;
    ProcessingLogger logger(ss);

    ProcessingStats stats1{111, 2000, 1000, "success", 3};
    ProcessingStats stats2{222, 4000, 2000, "corrupted", 0};

    logger.log(stats1);
    logger.log(stats2);
    logger.flush();

    std::stringstream log_stream(ss.str());
    std::string line;
    
    // Parse line 1
    std::getline(log_stream, line);
    auto j1 = nlohmann::json::parse(line);
    EXPECT_EQ(j1["ts_ns"], 111);
    EXPECT_DOUBLE_EQ(j1["throughput_mb_per_s"], 2.0);

    // Parse line 2
    std::getline(log_stream, line);
    auto j2 = nlohmann::json::parse(line);
    EXPECT_EQ(j2["ts_ns"], 222);
    EXPECT_EQ(j2["status"], "corrupted");
}
