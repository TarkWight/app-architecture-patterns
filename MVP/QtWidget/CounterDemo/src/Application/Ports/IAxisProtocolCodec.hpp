#ifndef IAXISPROTOCOLCODEC_HPP
#define IAXISPROTOCOLCODEC_HPP

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"

#include <cstdint>
#include <optional>
#include <cstddef>
#include <vector>

namespace application::ports {

class IAxisProtocolCodec {
  public:
    virtual ~IAxisProtocolCodec() = default;

    virtual std::vector<std::uint8_t> encodeCommand(const domain::AxisControlCommand &command) const = 0;

    virtual std::size_t telemetryFrameSize() const = 0;

    virtual std::size_t telemetryFrameHeaderSize() const = 0;

    virtual bool hasTelemetryFrameHeader(const std::vector<std::uint8_t> &bytes, std::size_t offset) const = 0;

    virtual bool isTelemetryFrameStructurallyValid(const std::vector<std::uint8_t> &bytes) const = 0;

    virtual bool isTelemetryFrameCrcConfirmed(const std::vector<std::uint8_t> &bytes) const = 0;

    virtual std::optional<domain::AxisTelemetrySample>
    decodeTelemetry(domain::AxisId axisId, const std::vector<std::uint8_t> &bytes, double timestampSeconds) const = 0;
};

} // namespace application::ports

#endif // IAXISPROTOCOLCODEC_HPP
