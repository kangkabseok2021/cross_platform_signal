#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include "postproc/BoundingBox.h"
#include "postproc/NMS.h"
#include "inference/CameraSimulator.h"
#include "inference/MockEngine.h"
#include "control/SortingQueue.h"
#include "control/MachineController.h"

TEST(PipelineTest, CameraSimulatorInitialization) {
    CameraSimulator camera;
    EXPECT_EQ(camera.get_current_frame_id(), 0);
}

TEST(PipelineTest, CameraSimulatorIncrement) {
    CameraSimulator camera;
    int id1 = camera.next_frame();
    int id2 = camera.next_frame();
    EXPECT_EQ(id1, 1);
    EXPECT_EQ(id2, 2);
    EXPECT_EQ(camera.get_current_frame_id(), 2);
}

TEST(PipelineTest, MockEngineRun) {
    MockEngine engine;
    std::vector<BoundingBox> mock_boxes = {
        BoundingBox{10.0f, 10.0f, 20.0f, 20.0f, 0.85f, 1}
    };
    engine.set_next_result(mock_boxes);

    auto result1 = engine.run(1);
    ASSERT_EQ(result1.size(), 1);
    EXPECT_FLOAT_EQ(result1[0].confidence, 0.85f);

    // Second run should return empty vector (as it is cleared)
    auto result2 = engine.run(2);
    EXPECT_TRUE(result2.empty());
}

TEST(PipelineTest, NMSFilteringIntegration) {
    std::vector<BoundingBox> raw_boxes = {
        BoundingBox{10.0f, 10.0f, 20.0f, 20.0f, 0.85f, 1},
        BoundingBox{11.0f, 10.0f, 20.0f, 20.0f, 0.95f, 1} // overlapping, higher confidence (kept)
    };
    
    // NMS suppression threshold = 0.5
    auto filtered = non_maximum_suppression(raw_boxes, 0.5f);
    ASSERT_EQ(filtered.size(), 1);
    EXPECT_FLOAT_EQ(filtered[0].confidence, 0.95f);
}

TEST(PipelineTest, SortingQueuePushIntegration) {
    SortingQueue<SortEvent, 64> queue;
    
    // Convert box to SortEvent and push
    SortEvent event;
    event.material = "PET_Plastic";
    event.confidence = 0.95f;
    event.x = 10.0f;
    event.y = 10.0f;
    event.w = 20.0f;
    event.h = 20.0f;
    event.timestamp = std::chrono::steady_clock::now();

    EXPECT_TRUE(queue.push(event));
    EXPECT_EQ(queue.size(), 1);

    SortEvent popped;
    EXPECT_TRUE(queue.pop(popped));
    EXPECT_EQ(popped.material, "PET_Plastic");
    EXPECT_FLOAT_EQ(popped.confidence, 0.95f);
}

TEST(PipelineTest, EndToEndFlow) {
    CameraSimulator camera;
    MockEngine engine;
    SortingQueue<SortEvent, 64> queue;
    MachineController controller(queue, 5);
    controller.start();

    // Setup overlapping raw boxes
    std::vector<BoundingBox> mock_boxes = {
        BoundingBox{10.0f, 10.0f, 20.0f, 20.0f, 0.85f, 0}, // PET, lower conf (suppressed)
        BoundingBox{11.0f, 10.0f, 20.0f, 20.0f, 0.95f, 0}  // PET, higher conf (kept)
    };
    engine.set_next_result(mock_boxes);

    int frame_id = camera.next_frame();
    auto raw = engine.run(frame_id);
    auto filtered = non_maximum_suppression(raw, 0.5f);

    for (const auto& box : filtered) {
        if (box.confidence >= 0.6f) {
            SortEvent event;
            event.material = "PET_Plastic";
            event.confidence = box.confidence;
            event.x = box.x;
            event.y = box.y;
            event.w = box.w;
            event.h = box.h;
            event.timestamp = std::chrono::steady_clock::now();
            queue.push(event);
        }
    }

    // Wait for the controller thread to process the event from queue
    std::this_thread::sleep_for(std::chrono::milliseconds(20));

    EXPECT_EQ(controller.get_sorted_count(), 1);
    controller.stop();
}
