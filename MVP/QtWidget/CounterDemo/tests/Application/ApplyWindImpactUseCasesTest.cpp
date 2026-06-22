#include "../../src/Application/UseCases/ApplyAngleOfAttackUseCase.hpp"
#include "../../src/Application/UseCases/ApplyBeaufortImpactUseCase.hpp"
#include "../../src/Application/UseCases/ApplyWindDirectionUseCase.hpp"

#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

void seedTargetImpact(application::session::SessionState &state) {
    state.setTargetStandImpact(domain::makeWindImpact(2.0, 90.0, 15.0));
}

TEST(ApplyBeaufortImpactUseCaseTest, WhenExecuted_ChangesOnlyBeaufort) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    application::useCases::ApplyBeaufortImpactUseCase useCase{state};

    const auto accepted = useCase.execute(domain::Beaufort::from(6.0));

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.beaufort.value(), 6.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.angleOfAttack.degrees(), 15.0);
}

TEST(ApplyBeaufortImpactUseCaseTest, HybridOperatorBeaufortCreatesOverrideState) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    state.setStandControlMode(domain::StandControlMode::Hybrid);
    application::useCases::ApplyBeaufortImpactUseCase useCase{state};

    const auto accepted = useCase.execute(domain::Beaufort::from(6.0));

    ASSERT_TRUE(accepted);
    ASSERT_TRUE(state.control().hybridBeaufortOverride.has_value());
    EXPECT_DOUBLE_EQ(state.control().hybridBeaufortOverride->startScenarioBeaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(state.control().hybridBeaufortOverride->operatorBeaufort.value(), 6.0);
    EXPECT_DOUBLE_EQ(state.control().targetStandImpact.beaufort.value(), 2.0);
}

TEST(ApplyBeaufortImpactUseCaseTest, AutomaticOperatorBeaufortDoesNotCreateOverrideState) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    state.setStandControlMode(domain::StandControlMode::PresetScenario);
    application::useCases::ApplyBeaufortImpactUseCase useCase{state};

    const auto accepted = useCase.execute(domain::Beaufort::from(6.0));

    EXPECT_FALSE(accepted);
    EXPECT_FALSE(state.control().hybridBeaufortOverride.has_value());
    EXPECT_DOUBLE_EQ(state.control().targetStandImpact.beaufort.value(), 2.0);
}

TEST(ApplyWindDirectionUseCaseTest, WhenExecuted_ChangesOnlyDirection) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    application::useCases::ApplyWindDirectionUseCase useCase{state};

    const auto accepted = useCase.execute(domain::WindDirection::from(270.0));

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.direction.degrees(), 270.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.angleOfAttack.degrees(), 15.0);
}

TEST(ApplyWindDirectionUseCaseTest, HybridStoresOperatorDirectionWithoutBeaufortOverride) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    state.setStandControlMode(domain::StandControlMode::Hybrid);
    application::useCases::ApplyWindDirectionUseCase useCase{state};

    const auto accepted = useCase.execute(domain::WindDirection::from(270.0));

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.control().hybridOperatorDirection.degrees(), 270.0);
    EXPECT_FALSE(state.control().hybridBeaufortOverride.has_value());
    EXPECT_DOUBLE_EQ(state.control().targetStandImpact.direction.degrees(), 90.0);
}

TEST(ApplyAngleOfAttackUseCaseTest, WhenExecuted_ChangesOnlyAngleOfAttack) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    application::useCases::ApplyAngleOfAttackUseCase useCase{state};

    const auto accepted = useCase.execute(domain::AngleOfAttack::from(45.0));

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.angleOfAttack.degrees(), 45.0);
}

TEST(ApplyAngleOfAttackUseCaseTest, HybridStoresOperatorAngleWithoutBeaufortOverride) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    state.setStandControlMode(domain::StandControlMode::Hybrid);
    application::useCases::ApplyAngleOfAttackUseCase useCase{state};

    const auto accepted = useCase.execute(domain::AngleOfAttack::from(45.0));

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.control().hybridOperatorAngleOfAttack.degrees(), 45.0);
    EXPECT_FALSE(state.control().hybridBeaufortOverride.has_value());
    EXPECT_DOUBLE_EQ(state.control().targetStandImpact.angleOfAttack.degrees(), 15.0);
}

TEST(ApplyAngleOfAttackUseCaseTest, WhenExecuted_DoesNotOverwriteWindDirection) {
    application::session::SessionState state{};
    state.setTargetStandImpact(domain::makeWindImpact(2.0, 90.0, 0.0));
    application::useCases::ApplyAngleOfAttackUseCase useCase{state};

    const auto accepted = useCase.execute(domain::AngleOfAttack::from(15.0));

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(state.get().control.targetStandImpact.angleOfAttack.degrees(), 15.0);
}

} // namespace
