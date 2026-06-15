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

    const auto accepted = useCase.execute(6.0);

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.beaufort.value(), 6.0);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.angleOfAttack.degrees(), 15.0);
}

TEST(ApplyWindDirectionUseCaseTest, WhenExecuted_ChangesOnlyDirection) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    application::useCases::ApplyWindDirectionUseCase useCase{state};

    const auto accepted = useCase.execute(270.0);

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.direction.degrees(), 270.0);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.angleOfAttack.degrees(), 15.0);
}

TEST(ApplyAngleOfAttackUseCaseTest, WhenExecuted_ChangesOnlyAngleOfAttack) {
    application::session::SessionState state{};
    seedTargetImpact(state);
    application::useCases::ApplyAngleOfAttackUseCase useCase{state};

    const auto accepted = useCase.execute(45.0);

    EXPECT_TRUE(accepted);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.beaufort.value(), 2.0);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.direction.degrees(), 90.0);
    EXPECT_DOUBLE_EQ(state.get().targetStandImpact.angleOfAttack.degrees(), 45.0);
}

} // namespace
