#ifndef IAXISPROTOCOLCODEC_HPP
#define IAXISPROTOCOLCODEC_HPP

#include "../../Domain/AxisControlCommand.hpp"
#include "../../Domain/AxisId.hpp"
#include "../../Domain/AxisTelemetrySample.hpp"

#include <cstdint>
#include <optional>
#include <vector>

namespace application::ports {

class IAxisProtocolCodec {
public:
    virtual ~IAxisProtocolCodec() = default;

    virtual std::vector<std::uint8_t> encodeCommand(const domain::AxisControlCommand &command) const = 0;

    virtual std::optional<domain::AxisTelemetrySample> decodeTelemetry(
        domain::AxisId axisId,
        const std::vector<std::uint8_t> &bytes,
        double timestampSeconds) const = 0;
};

} // namespace application::ports

#endif // IAXISPROTOCOLCODEC_HPP