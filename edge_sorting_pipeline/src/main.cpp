#include <iostream>
#include <chrono>
#include <thread>
#include <vector>
#include <memory>
#include <csignal>
#include <random>

#include "postproc/BoundingBox.h"
#include "postproc/NMS.h"
#include "inference/IInferenceEngine.h"
#include "inference/CameraSimulator.h"
#include "control/SortingQueue.h"
#include "control/MachineController.h"
#include "telemetry/TelemetrySender.h"

#ifdef BUILD_WITH_TFLITE
#include "inference/TFLiteEngine.h"
#endif

std::atomic<bool> g_running{true};
void signal_handler(int) {
    g_running = false;
}

class SimulatedInferenceEngine : public IInferenceEngine {
private:
    std::mt19937 rng_{42};
public:
    std::vector<BoundingBox> run(int /*frame_id*/) override {
        std::vector<BoundingBox> boxes;
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        // 20% chance of object presence in a simulated frame
        if (dist(rng_) < 0.2f) {
            std::uniform_int_distribution<int> num_boxes_dist(1, 3);
            int num_boxes = num_boxes_dist(rng_);
            
            for (int i = 0; i < num_boxes; ++i) {
                BoundingBox box;
                box.x = dist(rng_) * 100.0f;
                box.y = dist(rng_) * 100.0f;
                box.w = dist(rng_) * 50.0f + 10.0f;
                box.h = dist(rng_) * 50.0f + 10.0f;
                box.confidence = dist(rng_) * 0.5f + 0.5f;
                std::uniform_int_distribution<int> class_dist(0, 2);
                box.class_id = class_dist(rng_);
                boxes.push_back(box);
            }
        }
        return boxes;
    }
};

int main(int argc, char* argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::cout << "Starting High-Speed Edge Inference Pipeline for Optical Sorting...\n";

    std::string backend_host = "localhost";
    int backend_port = 8000;
    if (argc >= 2) backend_host = argv[1];
    if (argc >= 3) backend_port = std::stoi(argv[2]);

    std::cout << "Target telemetry backend: http://" << backend_host << ":" << backend_port << "/telemetry\n";

    SortingQueue<SortEvent, 64> queue;
    MachineController controller(queue, 50);
    controller.start();

    CameraSimulator camera(30.0);

    std::unique_ptr<IInferenceEngine> engine;
#ifdef BUILD_WITH_TFLITE
    std::cout << "Loading TensorFlow Lite model...\n";
    auto tflite_engine = std::make_unique<TFLiteEngine>();
    tflite_engine->load("models/efficientdet_lite0.tflite");
    engine = std::move(tflite_engine);
#else
    std::cout << "Running in SIMULATION mode (no TFLite)\n";
    engine = std::make_unique<SimulatedInferenceEngine>();
#endif

    TelemetrySender sender(backend_host, backend_port);
    const std::vector<std::string> materials = {"PET_Plastic", "Glass_Bottle", "HDPE_Container"};

    auto last_telemetry_time = std::chrono::steady_clock::now();

    while (g_running) {
        int frame_id = camera.next_frame();
        std::vector<BoundingBox> raw_boxes = engine->run(frame_id);
        std::vector<BoundingBox> filtered_boxes = non_maximum_suppression(raw_boxes, 0.5f);

        for (const auto& box : filtered_boxes) {
            if (box.confidence >= 0.6f) {
                SortEvent event;
                event.material = materials[box.class_id % materials.size()];
                event.confidence = box.confidence;
                event.x = box.x;
                event.y = box.y;
                event.w = box.w;
                event.h = box.h;
                event.timestamp = std::chrono::steady_clock::now();

                if (!queue.push(event)) {
                    std::cerr << "Warning: SortingQueue full, dropping event!\n";
                }
            }
        }

        auto now = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - last_telemetry_time).count() >= 500) {
            last_telemetry_time = now;
            
            TelemetryPayload payload;
            payload.sorted_count = controller.get_sorted_count();
            if (filtered_boxes.empty()) {
                payload.material = "None";
                payload.confidence = 0.0f;
            } else {
                auto max_it = std::max_element(filtered_boxes.begin(), filtered_boxes.end(), 
                    [](const BoundingBox& a, const BoundingBox& b) {
                        return a.confidence < b.confidence;
                    });
                payload.material = materials[max_it->class_id % materials.size()];
                payload.confidence = max_it->confidence;
            }
            
            sender.send(payload);
        }
    }

    std::cout << "Shutdown signal received. Stopping controller...\n";
    controller.stop();
    std::cout << "Shutdown complete. Total objects sorted: " << controller.get_sorted_count() << "\n";

    return 0;
}
