#include "../../src/Application/Services/ControlFunctionWorstCaseAnalyzer.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionStateData.hpp"

#include <gtest/gtest.h>

#include <limits>
#include <string>

namespace {

class DelayedPeakFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x >= 30.0 ? 6.0 : 1.0;
    }
};

class ConstantFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    explicit ConstantFunctionEngine(double value) : value(value) {
    }

    double eval(const std::string & /*expr*/, double /*x*/) const override {
        return value;
    }

  private:
    double value{0.0};
};

TEST(ControlFunctionWorstCaseAnalyzerTest, FindsPeakAfterPreviousEstimatedDurationInsideAnalysisWindow) {
    application::session::ProtocolStateData protocol{};
    protocol.testProtocol.testMode = domain::TestMode::Automatic;
    protocol.estimatedTestDuration = domain::DurationMinutes::required(1);
    application::session::ControlStateData control{};
    control.windImpact = domain::makeWindImpact(1.0, 0.0, 0.0);
    const DelayedPeakFunctionEngine engine{};
    const application::services::ControlFunctionWorstCaseAnalyzer analyzer{};

    const auto result = analyzer.analyze(protocol, control, engine, domain::DurationMinutes::required(48));

    EXPECT_TRUE(result.applicable);
    EXPECT_TRUE(result.usedFunction);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 6.0);
}

TEST(ControlFunctionWorstCaseAnalyzerTest, ManualModeIsNotApplicable) {
    application::session::ProtocolStateData protocol{};
    protocol.testProtocol.testMode = domain::TestMode::Manual;
    application::session::ControlStateData control{};
    control.windImpact = domain::makeWindImpact(2.0, 90.0, 10.0);
    const ConstantFunctionEngine engine{7.0};
    const application::services::ControlFunctionWorstCaseAnalyzer analyzer{};

    const auto result = analyzer.analyze(protocol, control, engine, domain::DurationMinutes::required(48));

    EXPECT_FALSE(result.applicable);
    EXPECT_FALSE(result.usedFunction);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 2.0);
}

TEST(ControlFunctionWorstCaseAnalyzerTest, HybridUsesOperatorDirectionAndAngle) {
    application::session::ProtocolStateData protocol{};
    protocol.testProtocol.testMode = domain::TestMode::Hybrid;
    application::session::ControlStateData control{};
    control.windImpact = domain::makeWindImpact(1.0, 0.0, 0.0);
    control.hybridOperatorDirection = domain::WindDirection::from(270.0);
    control.hybridOperatorAngleOfAttack = domain::AngleOfAttack::from(-30.0);
    const ConstantFunctionEngine engine{4.0};
    const application::services::ControlFunctionWorstCaseAnalyzer analyzer{};

    const auto result = analyzer.analyze(protocol, control, engine, domain::DurationMinutes::required(2));

    EXPECT_TRUE(result.usedFunction);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 4.0);
    EXPECT_DOUBLE_EQ(result.impact.direction.degrees(), 270.0);
    EXPECT_DOUBLE_EQ(result.impact.angleOfAttack.degrees(), -30.0);
}

TEST(ControlFunctionWorstCaseAnalyzerTest, HybridWorstCaseIncludesActiveOverrideOperatorBeaufort) {
    application::session::ProtocolStateData protocol{};
    protocol.testProtocol.testMode = domain::TestMode::Hybrid;
    application::session::ControlStateData control{};
    control.windImpact = domain::makeWindImpact(1.0, 0.0, 0.0);
    control.hybridBeaufortOverride = domain::HybridBeaufortOverridePolicy::startOverride(
        domain::Beaufort::from(3.0), domain::Beaufort::from(6.0), domain::ElapsedSeconds::from(0));
    const ConstantFunctionEngine engine{3.0};
    const application::services::ControlFunctionWorstCaseAnalyzer analyzer{};

    const auto result = analyzer.analyze(protocol, control, engine, domain::DurationMinutes::required(2));

    EXPECT_TRUE(result.usedFunction);
    EXPECT_DOUBLE_EQ(result.impact.beaufort.value(), 6.0);
}

TEST(ControlFunctionWorstCaseAnalyzerTest, RecordsDiagnosticWhenFunctionReturnsNonFiniteValue) {
    class InvalidFunctionEngine final : public application::ports::IFunctionEngine {
      public:
        double eval(const std::string & /*expr*/, double /*x*/) const override {
            return std::numeric_limits<double>::quiet_NaN();
        }
    };

    application::session::ProtocolStateData protocol{};
    protocol.testProtocol.testMode = domain::TestMode::Automatic;
    application::session::ControlStateData control{};
    const InvalidFunctionEngine engine{};
    const application::services::ControlFunctionWorstCaseAnalyzer analyzer{};

    const auto result = analyzer.analyze(protocol, control, engine, domain::DurationMinutes::required(1));

    EXPECT_FALSE(result.diagnostics.empty());
}

} // namespace
