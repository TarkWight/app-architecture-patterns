#include "LegacyAxisProtocolCodec.hpp"

#include <QByteArray>
#include <QtCore/QLoggingCategory>

#include <array>
#include <cstring>

namespace infrastructure::axisTcp {

namespace {

Q_LOGGING_CATEGORY(logCodec, "mvp.codec")

// CRC-16/CCITT table, poly 0x1021.
// Table-based implementation is used intentionally
constexpr std::array<std::uint16_t, 256> crc16Table{
    0,     4129,  8258,  12387, 16516, 20645, 24774, 28903, 33032, 37161, 41290, 45419, 49548, 53677, 57806, 61935,
    4657,  528,   12915, 8786,  21173, 17044, 29431, 25302, 37689, 33560, 45947, 41818, 54205, 50076, 62463, 58334,
    9314,  13379, 1056,  5121,  25830, 29895, 17572, 21637, 42346, 46411, 34088, 38153, 58862, 62927, 50604, 54669,
    13907, 9842,  5649,  1584,  30423, 26358, 22165, 18100, 46939, 42874, 38681, 34616, 63455, 59390, 55197, 51132,
    18628, 22757, 26758, 30887, 2112,  6241,  10242, 14371, 51660, 55789, 59790, 63919, 35144, 39273, 43274, 47403,
    23285, 19156, 31415, 27286, 6769,  2640,  14899, 10770, 56317, 52188, 64447, 60318, 39801, 35672, 47931, 43802,
    27814, 31879, 19684, 23749, 11298, 15363, 3168,  7233,  60846, 64911, 52716, 56781, 44330, 48395, 36200, 40265,
    32407, 28342, 24277, 20212, 15891, 11826, 7761,  3696,  65439, 61374, 57309, 53244, 48923, 44858, 40793, 36728,
    37256, 33193, 45514, 41451, 53516, 49453, 61774, 57711, 4224,  161,   12482, 8419,  20484, 16421, 28742, 24679,
    33721, 37784, 41979, 46042, 49981, 54044, 58239, 62302, 689,   4752,  8947,  13010, 16949, 21012, 25207, 29270,
    46570, 42443, 38312, 34185, 62830, 58703, 54572, 50445, 13538, 9411,  5280,  1153,  29798, 25671, 21540, 17413,
    42971, 47098, 34713, 38840, 59231, 63358, 50973, 55100, 9939,  14066, 1681,  5808,  26199, 30326, 17941, 22068,
    55628, 51565, 63758, 59695, 39368, 35305, 47498, 43435, 22596, 18533, 30726, 26663, 6336,  2273,  14466, 10403,
    52093, 56156, 60223, 64286, 35833, 39896, 43963, 48026, 19061, 23124, 27191, 31254, 2801,  6864,  10931, 14994,
    64814, 60687, 56684, 52557, 48554, 44427, 40424, 36297, 31782, 27655, 23652, 19525, 15522, 11395, 7392,  3265,
    61215, 65342, 53085, 57212, 44955, 49082, 36825, 40952, 28183, 32310, 20053, 24180, 11923, 16050, 3793,  7920};

constexpr std::size_t requestPacketWithoutCrcSize = 20;
constexpr std::size_t requestPacketWithCrcSize = 22;

constexpr std::size_t minTelemetryResponseSize = 28;

constexpr std::size_t positionOffset = 4;
constexpr std::size_t setPositionOffset = 8;
constexpr std::size_t torqueOffset = 12;
constexpr std::size_t setTorqueOffset = 16;
constexpr std::size_t voltageOffset = 20;
constexpr std::size_t currentOffset = 24;

QString axisName(domain::AxisId axisId) {
    return axisId == domain::axis0 ? QStringLiteral("axis0") : QStringLiteral("axis1");
}

QByteArray toByteArray(const std::vector<std::uint8_t> &bytes) {
    QByteArray out;
    out.reserve(static_cast<qsizetype>(bytes.size()));

    for (const auto byte : bytes) {
        out.append(static_cast<char>(byte));
    }

    return out;
}

QString bytesToHex(const std::vector<std::uint8_t> &bytes) {
    return QString::fromLatin1(toByteArray(bytes).toHex(' ').toUpper());
}

} // namespace

std::vector<std::uint8_t> LegacyAxisProtocolCodec::encodeCommand(const domain::AxisControlCommand &command) const {
    qCDebug(logCodec) << "encodeCommand input"
                      << "position=" << command.position
                      << "velocity=" << command.velocity
                      << "torque=" << command.torque
                      << "flags=" << command.cmd1 << command.cmd2 << command.cmd3 << command.cmd4;

    std::vector<std::uint8_t> packet{};
    packet.reserve(requestPacketWithCrcSize);

    packet.push_back(3);
    packet.push_back(130);
    packet.push_back(0);
    packet.push_back(12);

    appendFloatLe(packet, command.position);
    appendFloatLe(packet, command.velocity);
    appendFloatLe(packet, command.torque);

    packet.push_back(command.cmd1 ? 1U : 0U);
    packet.push_back(command.cmd2 ? 1U : 0U);
    packet.push_back(command.cmd3 ? 1U : 0U);
    packet.push_back(command.cmd4 ? 1U : 0U);

    if (packet.size() != requestPacketWithoutCrcSize) {
        qCWarning(logCodec) << "encodeCommand failed"
                            << "unexpectedSize=" << packet.size()
                            << "expected=" << requestPacketWithoutCrcSize;
        return {};
    }

    const std::uint16_t crc = crc16(packet);

    packet.push_back(static_cast<std::uint8_t>((crc >> 8) & 0xFFU));
    packet.push_back(static_cast<std::uint8_t>(crc & 0xFFU));

    qCInfo(logCodec) << "encodeCommand output"
                     << "size=" << packet.size()
                     << "crc=0x" << Qt::hex << crc << Qt::dec
                     << bytesToHex(packet);

    return packet;
}

std::optional<domain::AxisTelemetrySample>
LegacyAxisProtocolCodec::decodeTelemetry(domain::AxisId axisId, const std::vector<std::uint8_t> &bytes,
                                         double timestampSeconds) const {
    qCDebug(logCodec) << "decodeTelemetry input"
                      << axisName(axisId)
                      << "size=" << bytes.size()
                      << "timestamp=" << timestampSeconds
                      << bytesToHex(bytes);

    if (bytes.size() < minTelemetryResponseSize) {
        qCWarning(logCodec) << "decodeTelemetry rejected"
                            << axisName(axisId)
                            << "reason=short_response"
                            << "size=" << bytes.size()
                            << "min=" << minTelemetryResponseSize
                            << bytesToHex(bytes);
        return std::nullopt;
    }

    domain::AxisTelemetrySample sample{};
    sample.axisId = axisId;
    sample.timestampSeconds = timestampSeconds;

    sample.position = readFloatLe(bytes, positionOffset);
    sample.setPosition = readFloatLe(bytes, setPositionOffset);
    sample.torque = readFloatLe(bytes, torqueOffset);
    sample.setTorque = readFloatLe(bytes, setTorqueOffset);
    sample.voltage = readFloatLe(bytes, voltageOffset);
    sample.current = readFloatLe(bytes, currentOffset);

    sample.valid = true;

    qCInfo(logCodec) << "decodeTelemetry output"
                     << axisName(axisId)
                     << "position=" << sample.position
                     << "setPosition=" << sample.setPosition
                     << "torque=" << sample.torque
                     << "setTorque=" << sample.setTorque
                     << "voltage=" << sample.voltage
                     << "current=" << sample.current
                     << "valid=" << sample.valid;

    return sample;
}

void LegacyAxisProtocolCodec::appendFloatLe(std::vector<std::uint8_t> &buffer, float value) {
    static_assert(sizeof(float) == 4);

    std::array<std::uint8_t, sizeof(float)> raw{};
    std::memcpy(raw.data(), &value, sizeof(float));

    buffer.insert(buffer.end(), raw.begin(), raw.end());
}

float LegacyAxisProtocolCodec::readFloatLe(const std::vector<std::uint8_t> &buffer, std::size_t offset) {
    float value{0.0F};

    if (buffer.size() < offset + sizeof(float)) {
        qCWarning(logCodec) << "readFloatLe out of range"
                            << "offset=" << offset
                            << "size=" << buffer.size();
        return value;
    }

    std::memcpy(&value, buffer.data() + offset, sizeof(float));
    return value;
}

std::uint16_t LegacyAxisProtocolCodec::crc16(const std::vector<std::uint8_t> &buffer) {
    std::uint16_t crc = 0xFFFFU;

    for (const std::uint8_t byte : buffer) {
        crc = static_cast<std::uint16_t>(crc16Table[((crc >> 8U) ^ byte) & 0xFFU] ^ ((crc << 8U) & 0xFFFFU));
    }

    qCDebug(logCodec) << "crc16"
                      << "inputSize=" << buffer.size()
                      << "crc=0x" << Qt::hex << crc << Qt::dec;

    return crc;
}

} // namespace infrastructure::axisTcp