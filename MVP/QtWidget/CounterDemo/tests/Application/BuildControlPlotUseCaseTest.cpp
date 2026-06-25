#include "../../src/Application/UseCases/BuildControlPlotUseCase.hpp"
#include "../../src/Application/UseCases/SetStandControlModeUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/WindControlProfile.hpp"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

class LinearFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x;
    }
};

class OutOfRangeFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string & /*expr*/, double x) const override {
        return x < 1.0 ? -1.0 : 9.0;
    }
};

std::vector<domain::TestProtocolParameter> validDroneParameters() {
    return {
        {"uav_model", "Модель БАС", "BAS-1"},
        {"uav_total_weight_kg", "Полная масса", "3,5"},
        {"frontal_area_m2", "Фронтальная площадь", "0,2"},
        {"drag_coefficient", "Коэффициент сопротивления", "1,0"},
        {"equipment_current", "Ток оборудования", "1,0"},
        {"battery_capacity_mah", "Емкость", "18000"},
        {"battery_cell_count", "Число ячеек", "6"},
        {"battery_cell_voltage", "Напряжение ячейки", "3,8"},
        {"battery_discharge_rate_c", "C-rate", "6"},
        {"motor_count", "Количество двигателей", "4"},
        {"motor_max_thrust_kg", "Максимальная тяга", "2,0"},
        {"motor_peak_current_a", "Пиковый ток", "30"},
        {"motor_hover_current_a", "Ток висения", "6"},
    };
}

TEST(BuildControlPlotUseCaseTest, BuildsOneSecondWindControlProfileForCalculatedAutomaticDuration) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(48));
    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    const auto plot = useCase.execute();
    const auto &profile = state.control().controlProfile;

    EXPECT_EQ(profile.duration.value(), 48);
    EXPECT_DOUBLE_EQ(profile.sampleIntervalSeconds, domain::windControlProfileSampleIntervalSeconds);
    EXPECT_EQ(profile.samples.size(), static_cast<std::size_t>(48 * 60));
    EXPECT_EQ(plot.series.points.size(), profile.samples.size());

    EXPECT_DOUBLE_EQ(profile.samples.at(0).time.seconds(), 0.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(0).time.minutes(), 0.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).time.seconds(), 60.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).time.minutes(), 1.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).beaufort.value(), 1.0);

    EXPECT_DOUBLE_EQ(plot.x.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 48.0);
    EXPECT_DOUBLE_EQ(plot.y.min, domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(plot.y.max, domain::maxOperationalBeaufort);
}

TEST(BuildControlPlotUseCaseTest, ClampsFormulaOutputToOperationalBeaufortRange) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    const OutOfRangeFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    useCase.execute();
    const auto &profile = state.control().controlProfile;

    EXPECT_DOUBLE_EQ(profile.samples.at(0).beaufort.value(), domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).beaufort.value(), domain::maxOperationalBeaufort);
}

TEST(BuildControlPlotUseCaseTest, WhenAutoCalculatedUsesCurrentEstimatedDurationWithoutReadinessEstimation) {
    application::session::SessionState state{};
    const auto impact = domain::makeWindImpact(4.0, 0.0, 10.0);
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestTimeSource(domain::TestTimeSource::AutoCalculated);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(48));
    state.setTestProtocolDroneParameters(validDroneParameters());
    state.setWindImpact(impact);
    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    useCase.execute();

    EXPECT_EQ(state.readiness().status, application::session::ReadinessStatus::Unknown);
    EXPECT_EQ(state.protocol().estimatedTestDuration.value(), 48);
    EXPECT_EQ(state.control().controlProfile.duration.value(), 48);
}

TEST(BuildControlPlotUseCaseTest, ManualModeShowsEmptyControlGridUntilCommandsAreSent) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Manual);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(20));
    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    const auto plot = useCase.execute();

    EXPECT_TRUE(state.control().controlProfile.samples.empty());
    EXPECT_TRUE(plot.series.points.empty());
    EXPECT_TRUE(plot.seriesList.empty());
    EXPECT_DOUBLE_EQ(plot.x.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 20.0);
    EXPECT_DOUBLE_EQ(plot.y.min, domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(plot.y.max, domain::maxOperationalBeaufort);
}

TEST(BuildControlPlotUseCaseTest, PresetScenarioStandModeBuildsFormulaProfileForControlChart) {
    application::session::SessionState state{};
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    application::useCases::SetStandControlModeUseCase setModeUseCase{state};
    setModeUseCase.execute(domain::StandControlMode::PresetScenario);
    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    const auto plot = useCase.execute();

    EXPECT_EQ(state.protocol().testProtocol.testMode, domain::TestMode::Automatic);
    EXPECT_EQ(state.control().controlProfile.samples.size(), static_cast<std::size_t>(60));
    EXPECT_EQ(plot.series.points.size(), state.control().controlProfile.samples.size());
    EXPECT_FALSE(plot.series.points.empty());
}

TEST(BuildControlPlotUseCaseTest, UsesControlTraceAsTargetAndSafeCommandSeriesWhenAvailable) {
    application::session::SessionState state{};
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    state.appendControlTraceSample(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(0.0),
        .targetValue = domain::makeWindImpact(2.0, 0.0, 0.0),
        .safeCommandValue = domain::makeWindImpact(0.1, 0.0, 0.0),
    });
    state.appendControlTraceSample(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(1.0),
        .targetValue = domain::makeWindImpact(3.0, 0.0, 0.0),
        .safeCommandValue = domain::makeWindImpact(0.2, 0.0, 0.0),
    });

    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    const auto plot = useCase.execute();

    ASSERT_EQ(plot.seriesList.size(), 2U);
    EXPECT_TRUE(plot.series.points.empty());
    EXPECT_EQ(plot.seriesList.at(0).label, "Цель");
    EXPECT_EQ(plot.seriesList.at(1).label, "Безопасная команда");
    EXPECT_FALSE(plot.marker.visible);
    EXPECT_EQ(plot.x.label, "seconds");
    EXPECT_DOUBLE_EQ(plot.x.max, 10.0);
    EXPECT_DOUBLE_EQ(plot.x.step, 1.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(1).x, 1.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(1).y, 3.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(1).series.points.at(1).y, 0.2);
}

TEST(BuildControlPlotUseCaseTest, KeepsFormulaSeriesWhenControlTraceIsAvailableForScenarioMode) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Hybrid);
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(2));
    state.appendControlTraceSample(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(0.0),
        .targetValue = domain::makeWindImpact(2.0, 0.0, 0.0),
        .safeCommandValue = domain::makeWindImpact(0.1, 0.0, 0.0),
    });
    state.appendControlTraceSample(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(1.0),
        .targetValue = domain::makeWindImpact(3.0, 0.0, 0.0),
        .safeCommandValue = domain::makeWindImpact(0.2, 0.0, 0.0),
    });

    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    const auto plot = useCase.execute();

    ASSERT_EQ(plot.seriesList.size(), 3U);
    EXPECT_TRUE(plot.series.points.empty());
    EXPECT_EQ(plot.seriesList.at(0).label, "Формула");
    EXPECT_EQ(plot.seriesList.at(1).label, "Цель");
    EXPECT_EQ(plot.seriesList.at(2).label, "Безопасная команда");
    EXPECT_EQ(plot.x.label, "minutes");
    EXPECT_DOUBLE_EQ(plot.marker.x, 1.0 / 60.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(60).x, 1.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(0).series.points.at(60).y, 1.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(1).series.points.at(1).x, 1.0 / 60.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(1).series.points.at(1).y, 3.0);
    EXPECT_DOUBLE_EQ(plot.seriesList.at(2).series.points.at(1).y, 0.2);
}

TEST(BuildControlPlotUseCaseTest, RefreshFromStateKeepsExistingProfileAndUpdatesTraceSeries) {
    application::session::SessionState state{};
    state.setEstimatedTestDurationMinutes(domain::DurationMinutes::required(1));
    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};
    useCase.execute();

    const auto profileSize = state.control().controlProfile.samples.size();
    state.appendControlTraceSample(domain::ControlTraceSample{
        .time = domain::ControlTraceTime::fromSeconds(0.0),
        .targetValue = domain::makeWindImpact(2.0, 0.0, 0.0),
        .safeCommandValue = domain::makeWindImpact(0.1, 0.0, 0.0),
    });

    const auto plot = useCase.refreshFromState();

    EXPECT_EQ(state.control().controlProfile.samples.size(), profileSize);
    ASSERT_EQ(plot.seriesList.size(), 2U);
    EXPECT_FALSE(plot.marker.visible);
}

} // namespace
