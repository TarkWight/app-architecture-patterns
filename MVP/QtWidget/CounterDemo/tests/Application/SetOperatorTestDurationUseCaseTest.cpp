#include "../../src/Application/UseCases/SetOperatorTestDurationUseCase.hpp"

#include <gtest/gtest.h>

namespace {

TEST(SetOperatorTestDurationUseCaseTest, WhenExecuted_StoresOperatorDurationValueObject) {
    application::session::SessionState state{};
    application::useCases::SetOperatorTestDurationUseCase useCase{state};

    useCase.execute(domain::DurationMinutes::required(37));

    EXPECT_EQ(state.get().protocol.operatorTestDuration.value(), 37);
}

} // namespace
