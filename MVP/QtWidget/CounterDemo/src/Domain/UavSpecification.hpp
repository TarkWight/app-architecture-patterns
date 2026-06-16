#ifndef UAVSPECIFICATION_HPP
#define UAVSPECIFICATION_HPP

#include <string>

namespace domain {

struct UavFrameSpecification {
    std::string model{};
    std::string serialNumber{};
    double flightPayload{0.0};
    double dragRatioPercent{0.0};
    double equipmentCurrent{0.0};
    double operationRange{0.0};
    double maxSpeed{0.0};
    double maxAltitude{0.0};
    double maxFlightTime{0.0};
    std::string communicationType{};
};

struct PropellerSpecification {
    std::string model{};
    std::string configuration{};
    double diameter{0.0};
    double power{0.0};
    int maxRpm{0};
    int bladeCount{0};
    std::string material{};
};

struct BatterySpecification {
    std::string model{};
    std::string type{};
    int capacityMah{0};
    int cellCount{0};
    double dischargeRateC{0.0};
    double weight{0.0};
    double cellVoltage{0.0};
};

struct MotorSpecification {
    std::string model{};
    double shaftDiameter{0.0};
    int count{0};
    std::string manufacturer{};
    int kv{0};
    double maxThrustKg{0.0};
    double peakCurrentA{0.0};
    double weight{0.0};
};

struct UavSpecification {
    UavFrameSpecification frame{};
    PropellerSpecification propeller{};
    BatterySpecification battery{};
    MotorSpecification motor{};
    std::string firmwareVersion{};
};

} // namespace domain

#endif // UAVSPECIFICATION_HPP
