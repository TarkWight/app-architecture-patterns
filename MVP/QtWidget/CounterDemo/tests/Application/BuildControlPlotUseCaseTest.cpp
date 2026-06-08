#include "../../src/Application/UseCases/BuildControlPlotUseCase.hpp"

#include "../../src/Application/Ports/IFunctionEngine.hpp"
#include "../../src/Application/Session/SessionState.hpp"
#include "../../src/Domain/WindControlProfile.hpp"

#include <gtest/gtest.h>

#include <string>

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

TEST(BuildControlPlotUseCaseTest, BuildsOneSecondWindControlProfileForTemporaryMvpDuration) {
    application::session::SessionState state{};
    const LinearFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    const auto plot = useCase.execute();
    const auto &profile = state.get().controlProfile;

    EXPECT_EQ(profile.durationMinutes, domain::temporaryFormulaProfileDurationMinutes);
    EXPECT_DOUBLE_EQ(profile.sampleIntervalSeconds, domain::windControlProfileSampleIntervalSeconds);
    EXPECT_EQ(profile.samples.size(), static_cast<std::size_t>(48 * 60));
    EXPECT_EQ(plot.series.points.size(), profile.samples.size());

    EXPECT_DOUBLE_EQ(profile.samples.at(0).timeSeconds, 0.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(0).timeMinutes, 0.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).timeSeconds, 60.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).timeMinutes, 1.0);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).beaufort.value(), 1.0);

    EXPECT_DOUBLE_EQ(plot.x.min, 0.0);
    EXPECT_DOUBLE_EQ(plot.x.max, 48.0);
    EXPECT_DOUBLE_EQ(plot.y.min, domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(plot.y.max, domain::maxOperationalBeaufort);
}

TEST(BuildControlPlotUseCaseTest, ClampsFormulaOutputToOperationalBeaufortRange) {
    application::session::SessionState state{};
    const OutOfRangeFunctionEngine engine{};
    application::useCases::BuildControlPlotUseCase useCase{state, engine};

    useCase.execute();
    const auto &profile = state.get().controlProfile;

    EXPECT_DOUBLE_EQ(profile.samples.at(0).beaufort.value(), domain::minOperationalBeaufort);
    EXPECT_DOUBLE_EQ(profile.samples.at(60).beaufort.value(), domain::maxOperationalBeaufort);
}

} // namespace
