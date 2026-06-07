#include "../../src/Domain/AxisControlCommand.hpp"

#include <gtest/gtest.h>

namespace {

TEST(AxisControlCommandTest, StopCommandHasZeroMotionAndNoFlags) {
    const auto command = domain::stopAxisCommand();

    EXPECT_FLOAT_EQ(command.position, 0.0F);
    EXPECT_FLOAT_EQ(command.velocity, 0.0F);
    EXPECT_FLOAT_EQ(command.torque, 0.0F);
    EXPECT_FALSE(command.cmd1);
    EXPECT_FALSE(command.cmd2);
    EXPECT_FALSE(command.cmd3);
    EXPECT_FALSE(command.cmd4);
}

TEST(AxisControlCommandTest, Axis0CommandUsesAngleOfAttackAndAllFlags) {
    const auto profile = domain::makeWindProfile(3.5, 90.0, 12.0);

    const auto command = domain::axis0WindCommand(profile);

    EXPECT_FLOAT_EQ(command.position, 12.0F);
    EXPECT_FLOAT_EQ(command.velocity, domain::axisCommandVelocity);
    EXPECT_FLOAT_EQ(command.torque, 4.55F);
    EXPECT_TRUE(command.cmd1);
    EXPECT_TRUE(command.cmd2);
    EXPECT_TRUE(command.cmd3);
    EXPECT_TRUE(command.cmd4);
}

TEST(AxisControlCommandTest, Axis1CommandUsesDirectionAndOnlyFirstFlag) {
    const auto profile = domain::makeWindProfile(3.5, 90.0, 12.0);

    const auto command = domain::axis1WindCommand(profile);

    EXPECT_FLOAT_EQ(command.position, 90.0F);
    EXPECT_FLOAT_EQ(command.velocity, domain::axisCommandVelocity);
    EXPECT_FLOAT_EQ(command.torque, 3.5F);
    EXPECT_TRUE(command.cmd1);
    EXPECT_FALSE(command.cmd2);
    EXPECT_FALSE(command.cmd3);
    EXPECT_FALSE(command.cmd4);
}

TEST(AxisControlCommandTest, WindCommandsStopWhenImpactIsZero) {
    const auto profile = domain::makeWindProfile(0.0, 0.0, 0.0);

    EXPECT_FLOAT_EQ(domain::axis0WindCommand(profile).velocity, 0.0F);
    EXPECT_FLOAT_EQ(domain::axis1WindCommand(profile).velocity, 0.0F);
}

TEST(AxisControlCommandTest, BoundedCommandClampsVelocityAndTorque) {
    const auto command = domain::boundedAxisCommand(domain::AxisControlCommand{.velocity = 2.0F, .torque = 55.0F});

    EXPECT_FLOAT_EQ(command.velocity, 1.0F);
    EXPECT_FLOAT_EQ(command.torque, 50.0F);
}

} // namespace
