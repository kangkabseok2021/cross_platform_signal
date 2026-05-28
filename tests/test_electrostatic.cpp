#include <gtest/gtest.h>
#include "../electrostatic_monitor/src/ElectrostaticEngine.h"
#include <cmath>

TEST(ElectrostaticEngineTest, DecayAtZeroIsQ0) {
    EXPECT_DOUBLE_EQ(ElectrostaticEngine::computeDecay(5.0, 0.0, 50.0), 5.0);
}

TEST(ElectrostaticEngineTest, DecayAtTauIs1OverE) {
    EXPECT_NEAR(ElectrostaticEngine::computeDecay(5.0, 50.0, 50.0), 5.0 * std::exp(-1.0), 1e-12);
}

TEST(ElectrostaticEngineTest, DecayIsMonotonicallyDecreasing) {
    double last_val = ElectrostaticEngine::computeDecay(5.0, 0.0, 50.0);
    for (double t = 0.1; t <= 100.0; t += 0.1) {
        double val = ElectrostaticEngine::computeDecay(5.0, t, 50.0);
        EXPECT_LT(val, last_val);
        last_val = val;
    }
}

TEST(ElectrostaticEngineTest, DecayApproachesZero) {
    EXPECT_LT(ElectrostaticEngine::computeDecay(5.0, 1000.0, 50.0), 1e-6);
}

TEST(ElectrostaticEngineTest, NegativeCharge_DecayIsNegative) {
    EXPECT_LT(ElectrostaticEngine::computeDecay(-5.0, 10.0, 50.0), 0.0);
}

TEST(ElectrostaticEngineTest, ZeroCharge_AlwaysZero) {
    EXPECT_DOUBLE_EQ(ElectrostaticEngine::computeDecay(0.0, 10.0, 50.0), 0.0);
}

TEST(ElectrostaticEngineTest, AdcSample_WithinNoiseBounds) {
    AdcSimulator adc;
    adc.setTrueVoltage(5.0);
    for (int i = 0; i < 1000; ++i) {
        double s = adc.sample();
        EXPECT_NEAR(s, 5.0, 5 * 0.03 + 0.001); // 5 sigma + small drift tolerance
        adc.tick(0.001);
    }
}

TEST(ElectrostaticEngineTest, AdcCountsRoundTrip) {
    AdcSimulator adc;
    adc.setTrueVoltage(5.0);
    uint16_t counts = adc.sampleRaw();
    double v_back = AdcSimulator::countsToVoltage(counts);
    EXPECT_NEAR(v_back, adc.sample(), 2.0 * 0.6); // within ~ 2 LSB
}

TEST(ElectrostaticEngineTest, IonizerReducesTau_FasterDecay) {
    ElectrostaticEngine engine;
    engine.setInitialCharge(5.0);
    
    // Simulate 5 seconds without ionizer
    for (int i = 0; i < 50; ++i) engine.tick(0.1);
    double v1 = engine.voltage();
    EXPECT_GT(v1, 4.0);
    
    engine.deployIonizer();
    
    // Simulate 3 seconds with ionizer (tau=0.5s)
    for (int i = 0; i < 30; ++i) engine.tick(0.1);
    
    double v2 = engine.voltage();
    EXPECT_LT(v2, 5.0 * 0.05 + 0.5); // Should drop below 5% plus some noise buffer
}

TEST(ElectrostaticEngineTest, ThresholdExceeded_EventFired) {
    ElectrostaticEngine engine;
    engine.setInitialCharge(5.0);
    
    bool eventFired = false;
    double eventVoltage = 0.0;
    
    QObject::connect(&engine, &ElectrostaticEngine::dischargeEventOccurred,
                     [&](double v) { eventFired = true; eventVoltage = v; });
                     
    engine.tick(0.1); // 5.0 is immediately > 2.0
    EXPECT_TRUE(eventFired);
    EXPECT_GT(eventVoltage, 4.0);
}
