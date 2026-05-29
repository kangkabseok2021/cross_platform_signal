#include "DicomReader.h"
#include "DicomAnonymizer.h"
#include "ProcessingLogger.h"
#include "MetricsServer.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <chrono>
#include <csignal>
#include <atomic>
#include <thread>

namespace fs = std::filesystem;

std::atomic<bool> g_running{true};

void signal_handler(int) {
    g_running = false;
}

int main(int argc, char* argv[]) {
    // 1. Setup signal handlers
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 2. Parse arguments
    std::string input_dir;
    std::string output_dir;
    uint16_t metrics_port = 8080;
    std::string log_file = "processing.ndjson";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "--input-dir" || arg == "-i") && i + 1 < argc) {
            input_dir = argv[++i];
        } else if ((arg == "--output-dir" || arg == "-o") && i + 1 < argc) {
            output_dir = argv[++i];
        } else if ((arg == "--metrics-port" || arg == "-p") && i + 1 < argc) {
            metrics_port = static_cast<uint16_t>(std::stoi(argv[++i]));
        } else if ((arg == "--log-file" || arg == "-l") && i + 1 < argc) {
            log_file = argv[++i];
        }
    }

    if (input_dir.empty() || output_dir.empty()) {
        std::cerr << "Usage: " << argv[0] 
                  << " --input-dir <dir> --output-dir <dir> [--metrics-port <port>] [--log-file <file>]\n";
        return 1;
    }

    if (!fs::exists(input_dir)) {
        std::cerr << "Error: Input directory does not exist: " << input_dir << "\n";
        return 1;
    }

    if (!fs::exists(output_dir)) {
        fs::create_directories(output_dir);
    }

    // 3. Setup Telemetry Logger and Metrics Server
    std::ofstream log_stream(log_file, std::ios::app);
    if (!log_stream.is_open()) {
        std::cerr << "Error: Failed to open log file: " << log_file << "\n";
        return 1;
    }

    ProcessingLogger logger(log_stream);
    MetricsServer server;
    server.start(metrics_port);

    std::cout << "DICOM Pipeline started. Monitoring port " << metrics_port << "\n";

    // 4. Ingest and process DICOM files (Single-pass scan)
    size_t processed_count = 0;
    size_t success_count = 0;
    size_t corrupted_count = 0;

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (!g_running) {
            break;
        }

        if (entry.is_regular_file() && (entry.path().extension() == ".dcm" || entry.path().extension() == ".DCM")) {
            processed_count++;
            std::string filename = entry.path().filename().string();
            std::string in_path = entry.path().string();
            std::string out_path = (fs::path(output_dir) / filename).string();

            ProcessingStats stats;
            stats.ts_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            stats.file_size_bytes = fs::file_size(entry.path());

            auto t0 = std::chrono::high_resolution_clock::now();
            try {
                DicomReader reader;
                DicomFile file = reader.read(in_path);

                DicomAnonymizer anonymizer;
                AnonymizationResult anon_res = anonymizer.anonymize(in_path, out_path);

                auto t1 = std::chrono::high_resolution_clock::now();
                stats.duration_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

                if (anon_res.success) {
                    stats.status = "success";
                    stats.tags_scrubbed = anon_res.scrubbed_tags.size();
                    success_count++;
                    server.set_ready(); // First success sets ready status
                } else {
                    stats.status = "corrupted";
                    stats.tags_scrubbed = 0;
                    corrupted_count++;
                }
            } catch (const std::exception& e) {
                auto t1 = std::chrono::high_resolution_clock::now();
                stats.duration_us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
                stats.status = "corrupted";
                stats.tags_scrubbed = 0;
                corrupted_count++;
            }

            logger.log(stats);
            server.record(stats);
        }
    }

    std::cout << "Processed " << processed_count << " files: " 
              << success_count << " success, " << corrupted_count << " corrupted.\n";

    // 5. Block main thread until SIGINT / SIGTERM
    while (g_running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "Gracefully shutting down DICOM Pipeline.\n";
    server.stop();
    logger.flush();
    log_stream.close();

    return 0;
}
