#include <gtest/gtest.h>
#include "control/MachineController.h"
#include <chrono>
#include <thread>

TEST(ControllerTest, StartStop) {
    SortingQueue<SortEvent, 64> queue;
    MachineController controller(queue, 10);
    controller.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(5));
    controller.stop();
}

TEST(ControllerTest, SortedCountIncrements) {
    SortingQueue<SortEvent, 64> queue;
    MachineController controller(queue, 5);
    controller.start();

    SortEvent event{"PET_Plastic", 0.9f, 0, 0, 0, 0, std::chrono::steady_clock::now()};
    EXPECT_TRUE(queue.push(event));

    // Wait for the worker to process the event
    std::this_thread::sleep_for(std::chrono::milliseconds(25));
    
    EXPECT_EQ(controller.get_sorted_count(), 1);
    controller.stop();
}

TEST(ControllerTest, EjectorDelayTiming) {
    SortingQueue<SortEvent, 64> queue;
    // Set delay to 30ms
    MachineController controller(queue, 30);
    controller.start();

    SortEvent event{"PET_Plastic", 0.9f, 0, 0, 0, 0, std::chrono::steady_clock::now()};
    
    auto start = std::chrono::steady_clock::now();
    EXPECT_TRUE(queue.push(event));

    while (controller.get_sorted_count() == 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    auto end = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // Verify it took at least 30ms (give some tolerance like 28ms due to OS scheduler resolution)
    EXPECT_GE(elapsed, 28);

    controller.stop();
}

TEST(ControllerTest, EmptyQueueNoCrash) {
    SortingQueue<SortEvent, 64> queue;
    MachineController controller(queue, 5);
    controller.start();
    
    // Let it run empty
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    
    EXPECT_EQ(controller.get_sorted_count(), 0);
    controller.stop();
}

TEST(ControllerTest, StopTokenCancelsLoop) {
    SortingQueue<SortEvent, 64> queue;
    MachineController controller(queue, 100); // long delay
    controller.start();

    SortEvent event{"PET_Plastic", 0.9f, 0, 0, 0, 0, std::chrono::steady_clock::now()};
    EXPECT_TRUE(queue.push(event));
    
    // Stop immediately before delay finishes
    controller.stop();
    // Thread should join successfully and stop immediately without waiting for full delay
}
