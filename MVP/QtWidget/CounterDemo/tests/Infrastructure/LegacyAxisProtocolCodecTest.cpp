#include "../../src/Infrastructure/AxisTcp/LegacyAxisProtocolCodec.hpp"

#include <gtest/gtest.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

std::uint16_t crc16Ccitt(const std::vector<std::uint8_t> &bytes) {
    std::uint16_t crc = 0xFFFFU;

    for (const std::uint8_t byte : bytes) {
        crc ^= static_cast<std::uint16_t>(byte) << 8U;
        for (int bit = 0; bit < 8; ++bit) {
            if ((crc & 0x8000U) != 0U) {
                crc = static_cast<std::uint16_t>((crc << 1U) ^ 0x1021U);
            } else {
                crc = static_cast<std::uint16_t>(crc << 1U);
            }
        }
    }

    return crc;
}

std::array<std::uint8_t, 4> floatLe(float value) {
    std::array<std::uint8_t, 4> bytes{};
    std::memcpy(bytes.data(), &value, sizeof(float));
    return bytes;
}

void appendFloatLe(std::vector<std::uint8_t> &bytes, float value) {
    const auto raw = floatLe(value);
    bytes.insert(bytes.end(), raw.begin(), raw.end());
}

void expectFloatLeAt(const std::vector<std::uint8_t> &bytes, std::size_t offset, float value) {
    const auto expected = floatLe(value);
    for (std::size_t index = 0; index < expected.size(); ++index) {
        EXPECT_EQ(bytes.at(offset + index), expected.at(index));
    }
}

} // namespace

TEST(LegacyAxisProtocolCodecTest, EncodesLegacyCommand_WhenAxisCommandProvided_ReturnsTwentyTwoBytePacketWithValidCrc) {
    // Arrange
    const infrastructure::axisTcp::LegacyAxisProtocolCodec codec{};
    const domain::AxisControlCommand command{
        .position = 1.25F, .velocity = 0.06F, .torque = 3.5F, .cmd1 = true, .cmd2 = false, .cmd3 = true, .cmd4 = false};

    // Act
    const auto packet = codec.encodeCommand(command);

    // Assert
    ASSERT_EQ(packet.size(), 22U);
    EXPECT_EQ(packet.at(0), 0x03);
    EXPECT_EQ(packet.at(1), 0x82);
    EXPECT_EQ(packet.at(2), 0x00);
    EXPECT_EQ(packet.at(3), 0x0C);

    expectFloatLeAt(packet, 4, 1.25F);
    expectFloatLeAt(packet, 8, 0.06F);
    expectFloatLeAt(packet, 12, 3.5F);

    EXPECT_EQ(packet.at(16), 1U);
    EXPECT_EQ(packet.at(17), 0U);
    EXPECT_EQ(packet.at(18), 1U);
    EXPECT_EQ(packet.at(19), 0U);

    const std::vector<std::uint8_t> bytesWithoutCrc{packet.begin(), packet.begin() + 20};
    const std::uint16_t expectedCrc = crc16Ccitt(bytesWithoutCrc);
    EXPECT_EQ(packet.at(20), static_cast<std::uint8_t>((expectedCrc >> 8U) & 0xFFU));
    EXPECT_EQ(packet.at(21), static_cast<std::uint8_t>(expectedCrc & 0xFFU));
}

TEST(LegacyAxisProtocolCodecTest,
     EncodesZeroCommand_WhenStopCommandProvided_ReturnsTwentyTwoBytePacketWithZeroPayload) {
    // Arrange
    const infrastructure::axisTcp::LegacyAxisProtocolCodec codec{};
    const auto command = domain::stopAxisCommand();

    // Act
    const auto packet = codec.encodeCommand(command);

    // Assert
    ASSERT_EQ(packet.size(), 22U);
    EXPECT_EQ(packet.at(0), 0x03);
    EXPECT_EQ(packet.at(1), 0x82);
    EXPECT_EQ(packet.at(2), 0x00);
    EXPECT_EQ(packet.at(3), 0x0C);

    expectFloatLeAt(packet, 4, 0.0F);
    expectFloatLeAt(packet, 8, 0.0F);
    expectFloatLeAt(packet, 12, 0.0F);

    EXPECT_EQ(packet.at(16), 0U);
    EXPECT_EQ(packet.at(17), 0U);
    EXPECT_EQ(packet.at(18), 0U);
    EXPECT_EQ(packet.at(19), 0U);

    const std::vector<std::uint8_t> bytesWithoutCrc{packet.begin(), packet.begin() + 20};
    const std::uint16_t expectedCrc = crc16Ccitt(bytesWithoutCrc);
    EXPECT_EQ(packet.at(20), static_cast<std::uint8_t>((expectedCrc >> 8U) & 0xFFU));
    EXPECT_EQ(packet.at(21), static_cast<std::uint8_t>(expectedCrc & 0xFFU));
}

TEST(LegacyAxisProtocolCodecTest, DecodesTelemetryFrame_WhenThirtyByteFrameProvided_ReturnsExpectedFields) {
    // Arrange
    const infrastructure::axisTcp::LegacyAxisProtocolCodec codec{};
    std::vector<std::uint8_t> frame{0x03, 0x82, 0x00, 0x16};
    appendFloatLe(frame, 1.25F);
    appendFloatLe(frame, 90.0F);
    appendFloatLe(frame, 3.5F);
    appendFloatLe(frame, 4.5F);
    appendFloatLe(frame, 28.3F);
    appendFloatLe(frame, 0.75F);
    frame.push_back(0x00);
    frame.push_back(0x00);
    ASSERT_EQ(frame.size(), 30U);

    // Act
    const auto sample = codec.decodeTelemetry(domain::axis1, frame, 12.5);

    // Assert
    ASSERT_TRUE(sample.has_value());
    EXPECT_EQ(sample->axisId, domain::axis1);
    EXPECT_DOUBLE_EQ(sample->timestampSeconds, 12.5);
    EXPECT_TRUE(sample->valid);
    EXPECT_FLOAT_EQ(sample->position, 1.25F);
    EXPECT_FLOAT_EQ(sample->setPosition, 90.0F);
    EXPECT_FLOAT_EQ(sample->torque, 3.5F);
    EXPECT_FLOAT_EQ(sample->setTorque, 4.5F);
    EXPECT_FLOAT_EQ(sample->voltage, 28.3F);
    EXPECT_FLOAT_EQ(sample->current, 0.75F);
}
