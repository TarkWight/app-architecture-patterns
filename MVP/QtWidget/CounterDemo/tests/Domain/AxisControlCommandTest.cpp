#include "../../src/Domain/AxisControlCommand.hpp"
#include "../../src/Domain/StandCommandMapper.hpp"

#include <gtest/gtest.h>

namespace {

struct BeaufortTorqueExpectation {
    double beaufort;
    float axis0Torque;
    float axis1Torque;
};

class BeaufortCommandMappingTest : public testing::TestWithParam<BeaufortTorqueExpectation> {};

class DirectionCommandMappingTest : public testing::TestWithParam<double> {};

void expectAxis0ActiveCommand(const domain::AxisControlCommand &command, float expectedTorque) {
    EXPECT_FLOAT_EQ(command.position, 0.0F);
    EXPECT_FLOAT_EQ(command.velocity, domain::axisCommandVelocity);
    EXPECT_FLOAT_EQ(command.torque, expectedTorque);
    EXPECT_TRUE(command.cmd1);
    EXPECT_TRUE(command.cmd2);
    EXPECT_TRUE(command.cmd3);
    EXPECT_TRUE(command.cmd4);
}

void expectAxis1ActiveCommand(const domain::AxisControlCommand &command, float expectedPosition, float expectedTorque) {
    EXPECT_FLOAT_EQ(command.position, expectedPosition);
    EXPECT_FLOAT_EQ(command.velocity, domain::axisCommandVelocity);
    EXPECT_FLOAT_EQ(command.torque, expectedTorque);
    EXPECT_TRUE(command.cmd1);
    EXPECT_FALSE(command.cmd2);
    EXPECT_FALSE(command.cmd3);
    EXPECT_FALSE(command.cmd4);
}

TEST(AxisControlCommandTest, StopCommandHasZeroMotionAndNoFlags) {
    // Arrange / Act
    const auto command = domain::stopAxisCommand();

    // Assert
    EXPECT_FLOAT_EQ(command.position, 0.0F);
    EXPECT_FLOAT_EQ(command.velocity, 0.0F);
    EXPECT_FLOAT_EQ(command.torque, 0.0F);
    EXPECT_FALSE(command.cmd1);
    EXPECT_FALSE(command.cmd2);
    EXPECT_FALSE(command.cmd3);
    EXPECT_FALSE(command.cmd4);
}

TEST_P(BeaufortCommandMappingTest, MapsBeaufortToAxisTorque_WhenBeaufortProvided_ReturnsRecoveredTorqueCommands) {
    // Arrange
    const auto expected = GetParam();
    const auto impact = domain::makeWindImpact(expected.beaufort, 0.0, 0.0);

    // Act
    const auto commands = domain::StandCommandMapper::map(impact);

    // Assert
    expectAxis0ActiveCommand(commands.axis0, expected.axis0Torque);
    expectAxis1ActiveCommand(commands.axis1, 0.0F, expected.axis1Torque);
}

INSTANTIATE_TEST_SUITE_P(
    BeaufortReferenceTable, BeaufortCommandMappingTest,
    testing::Values(BeaufortTorqueExpectation{0.0, 0.0F, 0.0F}, BeaufortTorqueExpectation{1.0, 6.5F, 5.0F},
                    BeaufortTorqueExpectation{2.0, 13.0F, 10.0F}, BeaufortTorqueExpectation{3.0, 19.5F, 15.0F},
                    BeaufortTorqueExpectation{4.0, 26.0F, 20.0F}, BeaufortTorqueExpectation{5.0, 32.5F, 25.0F},
                    BeaufortTorqueExpectation{6.0, 39.0F, 30.0F}, BeaufortTorqueExpectation{7.0, 45.5F, 35.0F}));

TEST(AxisControlCommandTest, MapsHistoricalEtalonBeaufort_WhenBeaufortIsSixPointNine_ReturnsRecoveredTorques) {
    // Arrange
    const auto impact = domain::makeWindImpact(6.9, 0.0, 0.0);

    // Act
    const auto commands = domain::StandCommandMapper::map(impact);

    // Assert
    EXPECT_NEAR(commands.axis0.torque, 44.85F, 0.0001F);
    EXPECT_FLOAT_EQ(commands.axis1.torque, 34.5F);
}

TEST(StandCommandMapperTest, WhenBeaufortIsApplied_ReturnsExpectedTorqueCommands) {
    const auto impact = domain::makeWindImpact(3.0, 0.0, 0.0);

    const auto commands = domain::StandCommandMapper::map(impact);

    EXPECT_FLOAT_EQ(commands.axis0.torque, 19.5F);
    EXPECT_FLOAT_EQ(commands.axis1.torque, 15.0F);
}

TEST(StandCommandMapperTest, WhenDirectionIsApplied_ReturnsAxis1Position) {
    const auto impact = domain::makeWindImpact(0.0, 90.0, 0.0);

    const auto commands = domain::StandCommandMapper::map(impact);

    EXPECT_FLOAT_EQ(commands.axis1.position, 90.0F);
}

TEST(StandCommandMapperTest, WhenAngleOfAttackIsApplied_TargetsAxis1SectorInsteadOfOverwritingDirection) {
    const auto impact = domain::makeWindImpact(0.0, 90.0, 15.0);

    const auto commands = domain::StandCommandMapper::map(impact);

    EXPECT_FLOAT_EQ(commands.axis0.position, 0.0F);
    EXPECT_FLOAT_EQ(commands.axis1.position, 97.5F);
}

TEST_P(DirectionCommandMappingTest, MapsDirectionToAxis1Position_WhenWindRoseDirectionProvided_ReturnsSamePosition) {
    // Arrange
    const double direction = GetParam();
    const auto impact = domain::makeWindImpact(0.0, direction, 0.0);

    // Act
    const auto commands = domain::StandCommandMapper::map(impact);

    // Assert
    expectAxis1ActiveCommand(commands.axis1, static_cast<float>(direction), 0.0F);
}

INSTANTIATE_TEST_SUITE_P(SixteenPointWindRose, DirectionCommandMappingTest,
                         testing::Values(0.0, 22.5, 45.0, 67.5, 90.0, 112.5, 135.0, 157.5, 180.0, 202.5, 225.0, 247.5,
                                         270.0, 292.5, 315.0, 337.5));

TEST(AxisControlCommandTest, MapsWindImpact_WhenBeaufortSevenAndDirectionEast_ReturnsCombinedAxisCommands) {
    // Arrange
    const auto impact = domain::makeWindImpact(7.0, 90.0, 0.0);

    // Act
    const auto commands = domain::StandCommandMapper::map(impact);

    // Assert
    expectAxis0ActiveCommand(commands.axis0, 45.5F);
    expectAxis1ActiveCommand(commands.axis1, 90.0F, 35.0F);
}

TEST(AxisControlCommandTest, BoundedCommandClampsVelocityAndTorque) {
    // Arrange / Act
    const auto command = domain::boundedAxisCommand(domain::AxisControlCommand{.velocity = 2.0F, .torque = 55.0F});

    // Assert
    EXPECT_FLOAT_EQ(command.velocity, 1.0F);
    EXPECT_FLOAT_EQ(command.torque, 50.0F);
}

} // namespace
