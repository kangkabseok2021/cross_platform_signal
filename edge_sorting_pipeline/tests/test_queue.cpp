#include <gtest/gtest.h>
#include "control/SortingQueue.h"
#include <thread>
#include <vector>

TEST(QueueTest, PushPopRoundTrip) {
    SortingQueue<int, 4> queue;
    EXPECT_TRUE(queue.empty());
    
    EXPECT_TRUE(queue.push(42));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(queue.size(), 1);

    int val = 0;
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 42);
    EXPECT_TRUE(queue.empty());
}

TEST(QueueTest, FIFOOrder) {
    SortingQueue<int, 5> queue;
    EXPECT_TRUE(queue.push(10));
    EXPECT_TRUE(queue.push(20));
    EXPECT_TRUE(queue.push(30));

    int val = 0;
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 10);
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 20);
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 30);
}

TEST(QueueTest, FullDetection) {
    // A ring buffer of capacity N can hold at most N - 1 elements to distinguish full from empty.
    SortingQueue<int, 4> queue;
    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    EXPECT_TRUE(queue.push(3));
    // The 4th push must fail because (head + 1) % 4 == tail
    EXPECT_FALSE(queue.push(4));
    EXPECT_EQ(queue.size(), 3);
}

TEST(QueueTest, EmptyDetection) {
    SortingQueue<int, 4> queue;
    int val = 0;
    EXPECT_FALSE(queue.pop(val));
}

TEST(QueueTest, SizeCheck) {
    SortingQueue<int, 8> queue;
    EXPECT_EQ(queue.size(), 0);
    queue.push(1);
    queue.push(2);
    EXPECT_EQ(queue.size(), 2);
    int val;
    queue.pop(val);
    EXPECT_EQ(queue.size(), 1);
}

TEST(QueueTest, WrapAround) {
    SortingQueue<int, 3> queue; // Max 2 elements
    
    // Cycle 1
    EXPECT_TRUE(queue.push(1));
    EXPECT_TRUE(queue.push(2));
    int val;
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 1);

    // Cycle 2 (causes wrap around)
    EXPECT_TRUE(queue.push(3));
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 2);
    EXPECT_TRUE(queue.pop(val));
    EXPECT_EQ(val, 3);
}

TEST(QueueTest, MultithreadedProducerConsumer) {
    SortingQueue<int, 128> queue;
    const int count = 1000;
    
    std::thread producer([&]() {
        for (int i = 0; i < count; ) {
            if (queue.push(i)) {
                ++i;
            } else {
                std::this_thread::yield();
            }
        }
    });

    std::vector<int> popped;
    std::thread consumer([&]() {
        for (int i = 0; i < count; ) {
            int val;
            if (queue.pop(val)) {
                popped.push_back(val);
                ++i;
            } else {
                std::this_thread::yield();
            }
        }
    });

    producer.join();
    consumer.join();

    ASSERT_EQ(popped.size(), count);
    for (int i = 0; i < count; ++i) {
        EXPECT_EQ(popped[i], i);
    }
}
