#include <gtest/gtest.h>
#include "telemetry/TelemetrySender.h"

TEST(TelemetryTest, ConnectionFailureHandling) {
    // 9999 is highly likely to be closed. Sender should fail gracefully (return false, no exception)
    TelemetrySender sender("localhost", 9999);
    TelemetryPayload payload{"PET_Plastic", 0.85f, 5};
    
    // Should return false and not throw
    bool success = false;
    EXPECT_NO_THROW({
        success = sender.send(payload);
    });
    EXPECT_FALSE(success);
}
