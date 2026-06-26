#include "../../src/Application/Services/ReadinessDiagnosticMessageBuilder.hpp"

#include <gtest/gtest.h>

#include <array>
#include <string>

namespace {

domain::TestDurationDiagnosticValues diagnosticValues() {
    domain::TestDurationDiagnosticValues values{};
    values.totalMassKg = 3.5;
    values.frontalAreaM2 = 0.35;
    values.dragCoefficient = 1.1;
    values.requiredThrustKg = 9.4;
    values.availableThrustKg = 8.0;
    values.estimatedCurrentA = 125.0;
    values.maxMotorCurrentA = 120.0;
    values.maxBatteryCurrentA = 108.0;
    return values;
}

application::session::ReadinessStateData readinessWith(application::session::ReadinessStatus status) {
    application::session::ReadinessStateData readiness{};
    readiness.status = status;
    readiness.values = diagnosticValues();
    return readiness;
}

bool contains(const std::string &text, const std::string &fragment) {
    return text.find(fragment) != std::string::npos;
}

} // namespace

TEST(ReadinessDiagnosticMessageBuilderTest, EveryTestDurationDiagnosticCodeHasOperatorMessage) {
    using domain::TestDurationDiagnosticCode;

    constexpr std::array codes{
        TestDurationDiagnosticCode::TotalMassMissing,
        TestDurationDiagnosticCode::TotalMassEstimated,
        TestDurationDiagnosticCode::BatteryCapacityMissing,
        TestDurationDiagnosticCode::BatteryCellCountMissing,
        TestDurationDiagnosticCode::BatteryCellVoltageMissing,
        TestDurationDiagnosticCode::MotorCountMissing,
        TestDurationDiagnosticCode::MotorPeakCurrentMissing,
        TestDurationDiagnosticCode::FrontalAreaFallbackUsed,
        TestDurationDiagnosticCode::DragCoefficientFallbackUsed,
        TestDurationDiagnosticCode::MotorMaxThrustMissing,
        TestDurationDiagnosticCode::RequiredThrustExceedsAvailable,
        TestDurationDiagnosticCode::MotorPeakCurrentExceeded,
        TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded,
        TestDurationDiagnosticCode::AngleOfAttackClampedByMinCos,
    };

    for (const auto code : codes) {
        const auto message =
            application::services::ReadinessDiagnosticMessageBuilder::messageForDiagnostic(code, diagnosticValues());
        EXPECT_FALSE(message.empty());
        EXPECT_FALSE(contains(message, "Неизвестная диагностика")) << message;
    }
}

TEST(ReadinessDiagnosticMessageBuilderTest, OkProducesSuccessSummaryWithoutDetails) {
    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(
        readinessWith(application::session::ReadinessStatus::Ok));

    EXPECT_EQ(message.summary, "Расчёт готовности выполнен. Испытание допустимо.");
    EXPECT_TRUE(message.details.empty());
    EXPECT_EQ(message.toDisplayText(), message.summary);
}

TEST(ReadinessDiagnosticMessageBuilderTest, WarningProducesWarningSummaryAndDetails) {
    auto readiness = readinessWith(application::session::ReadinessStatus::Warning);
    readiness.warnings.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed});

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(readiness);

    ASSERT_EQ(message.details.size(), 1U);
    EXPECT_EQ(message.summary, "Расчёт готовности выполнен. Есть предупреждения.");
    EXPECT_TRUE(contains(message.details.front(), "Фронтальная площадь"));
}

TEST(ReadinessDiagnosticMessageBuilderTest, DangerousProducesDangerSummaryAndDetails) {
    auto readiness = readinessWith(application::session::ReadinessStatus::Dangerous);
    readiness.errors.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable});

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(readiness);

    ASSERT_EQ(message.details.size(), 1U);
    EXPECT_EQ(message.summary, "Испытание потенциально опасно.");
    EXPECT_TRUE(contains(message.details.front(), "превышает доступную тягу"));
}

TEST(ReadinessDiagnosticMessageBuilderTest, FailedProducesFailureSummaryAndDetails) {
    auto readiness = readinessWith(application::session::ReadinessStatus::Failed);
    readiness.errors.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::BatteryCapacityMissing});

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(readiness);

    ASSERT_EQ(message.details.size(), 1U);
    EXPECT_EQ(message.summary, "Расчёт готовности невозможен. Испытание потенциально опасно.");
    EXPECT_TRUE(contains(message.details.front(), "Ёмкость АКБ"));
}

TEST(ReadinessDiagnosticMessageBuilderTest, MessagesIncludeDiagnosticNumericValuesWhereAvailable) {
    const auto thrustMessage = application::services::ReadinessDiagnosticMessageBuilder::messageForDiagnostic(
        domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable, diagnosticValues());
    const auto currentMessage = application::services::ReadinessDiagnosticMessageBuilder::messageForDiagnostic(
        domain::TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded, diagnosticValues());
    const auto fallbackMessage = application::services::ReadinessDiagnosticMessageBuilder::messageForDiagnostic(
        domain::TestDurationDiagnosticCode::DragCoefficientFallbackUsed, diagnosticValues());

    EXPECT_TRUE(contains(thrustMessage, "9.40"));
    EXPECT_TRUE(contains(thrustMessage, "8.00"));
    EXPECT_TRUE(contains(currentMessage, "125.00"));
    EXPECT_TRUE(contains(currentMessage, "108.00"));
    EXPECT_TRUE(contains(fallbackMessage, "1.10"));
}

TEST(ReadinessDiagnosticMessageBuilderTest, WorstCaseReadinessShowsScenarioImpact) {
    auto readiness = readinessWith(application::session::ReadinessStatus::Warning);
    readiness.hasCalculatedForImpact = true;
    readiness.calculatedForWorstCaseScenario = true;
    readiness.calculatedForImpact = domain::makeWindImpact(6.0, 90.0, -15.0);

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(readiness);

    ASSERT_FALSE(message.details.empty());
    EXPECT_TRUE(contains(message.details.front(), "худшему участку сценария"));
    EXPECT_TRUE(contains(message.details.front(), "Бофорт 6.0"));
    EXPECT_TRUE(contains(message.details.front(), "угол атаки -15.0°"));
}

TEST(ReadinessDiagnosticMessageBuilderTest, IncludesSafeLimitsWhenAvailable) {
    auto readiness = readinessWith(application::session::ReadinessStatus::Ok);
    readiness.safeLimits.status = domain::SafeWindImpactLimitStatus::Available;
    readiness.safeLimits.maxSafeBeaufort = 6.5;
    readiness.safeLimits.maxSafeAbsAngleOfAttack = 45.0;

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(readiness);
    const auto text = message.toDisplayText();

    EXPECT_TRUE(contains(text, "Безопасный предел по Бофорту: 6.5"));
    EXPECT_TRUE(contains(text, "Безопасный предел угла атаки: ±45°"));
}

TEST(ReadinessDiagnosticMessageBuilderTest, ShowsReasonWhenSafeLimitsUnavailable) {
    auto readiness = readinessWith(application::session::ReadinessStatus::Failed);
    readiness.safeLimits.status = domain::SafeWindImpactLimitStatus::Unavailable;
    readiness.safeLimits.diagnostics.push_back(
        domain::TestDurationDiagnostic{.code = domain::TestDurationDiagnosticCode::BatteryCapacityMissing});

    const auto message = application::services::ReadinessDiagnosticMessageBuilder::build(readiness);
    const auto text = message.toDisplayText();

    EXPECT_TRUE(contains(text, "Безопасные пределы воздействия недоступны"));
}
