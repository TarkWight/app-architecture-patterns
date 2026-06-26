#ifndef LEGACYAXISPROTOCOLCODEC_HPP
#define LEGACYAXISPROTOCOLCODEC_HPP

#include "../../Application/Ports/IAxisProtocolCodec.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace infrastructure::axisTcp {

class LegacyAxisProtocolCodec final : public application::ports::IAxisProtocolCodec {
  public:
    std::vector<std::uint8_t> encodeCommand(const domain::AxisControlCommand &command) const override;

    std::size_t telemetryFrameSize() const override;

    std::size_t telemetryFrameHeaderSize() const override;

    bool hasTelemetryFrameHeader(const std::vector<std::uint8_t> &bytes, std::size_t offset) const override;

    bool isTelemetryFrameStructurallyValid(const std::vector<std::uint8_t> &bytes) const override;

    bool isTelemetryFrameCrcConfirmed(const std::vector<std::uint8_t> &bytes) const override;

    std::optional<domain::AxisTelemetrySample> decodeTelemetry(domain::AxisId axisId,
                                                               const std::vector<std::uint8_t> &bytes,
                                                               double timestampSeconds) const override;

  private:
    static void appendFloatLe(std::vector<std::uint8_t> &buffer, float value);
    static float readFloatLe(const std::vector<std::uint8_t> &buffer, std::size_t offset);
    static std::uint16_t crc16(const std::vector<std::uint8_t> &buffer);
};

} // namespace infrastructure::axisTcp

#endif // LEGACYAXISPROTOCOLCODEC_HPP
