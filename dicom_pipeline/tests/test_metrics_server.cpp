#include <gtest/gtest.h>
#include "MetricsServer.h"
#include <httplib.h>

TEST(MetricsServerTest, HealthAndReadyEndpoints) {
    MetricsServer server;
    server.start(18080);

    httplib::Client cli("localhost", 18080);
    
    // Test /health
    auto res_health = cli.Get("/health");
    ASSERT_TRUE(res_health);
    EXPECT_EQ(res_health->status, 200);
    EXPECT_EQ(res_health->get_header_value("Content-Type"), "application/json");
    EXPECT_EQ(res_health->body, "{\"status\":\"ok\"}");

    // Test /ready is initially 503
    auto res_ready_init = cli.Get("/ready");
    ASSERT_TRUE(res_ready_init);
    EXPECT_EQ(res_ready_init->status, 503);
    EXPECT_EQ(res_ready_init->body, "{\"status\":\"initializing\"}");

    // Set ready and check again
    server.set_ready();
    auto res_ready_final = cli.Get("/ready");
    ASSERT_TRUE(res_ready_final);
    EXPECT_EQ(res_ready_final->status, 200);
    EXPECT_EQ(res_ready_final->body, "{\"status\":\"ready\"}");

    server.stop();
}

TEST(MetricsServerTest, RecordAndScrapeMetrics) {
    MetricsServer server;
    server.start(18081);

    // Record stats
    ProcessingStats stats1{123, 1000000, 50000, "success", 6}; // 0.05 seconds
    ProcessingStats stats2{124, 2000000, 2000, "success", 6};  // 0.002 seconds (fits <= 0.005 bucket)
    ProcessingStats stats3{125, 0, 0, "corrupted", 0};

    server.record(stats1);
    server.record(stats2);
    server.record(stats3);

    httplib::Client cli("localhost", 18081);
    auto res = cli.Get("/metrics");
    ASSERT_TRUE(res);
    EXPECT_EQ(res->status, 200);
    EXPECT_EQ(res->get_header_value("Content-Type"), "text/plain; version=0.0.4");

    std::string body = res->body;
    
    // Check success counter
    EXPECT_NE(body.find("dicom_files_processed_total{result=\"success\"} 2"), std::string::npos);
    // Check corrupted counter
    EXPECT_NE(body.find("dicom_files_processed_total{result=\"corrupted\"} 1"), std::string::npos);

    // Check histogram buckets
    // le="0.001" count should be 0
    EXPECT_NE(body.find("dicom_processing_seconds_bucket{le=\"0.001\"} 0"), std::string::npos);
    // le="0.005" count should be 1 (stats2 duration is 0.002s)
    EXPECT_NE(body.find("dicom_processing_seconds_bucket{le=\"0.005\"} 1"), std::string::npos);
    // le="0.05" count should be 2 (stats1 duration is 0.05s)
    EXPECT_NE(body.find("dicom_processing_seconds_bucket{le=\"0.05\"} 2"), std::string::npos);
    // le="+Inf" count should be 2
    EXPECT_NE(body.find("dicom_processing_seconds_bucket{le=\"+Inf\"} 2"), std::string::npos);

    // Check sum (0.05 + 0.002 = 0.052)
    EXPECT_NE(body.find("dicom_processing_seconds_sum 0.052000"), std::string::npos);
    // Check total count
    EXPECT_NE(body.find("dicom_processing_seconds_count 2"), std::string::npos);

    server.stop();
}
