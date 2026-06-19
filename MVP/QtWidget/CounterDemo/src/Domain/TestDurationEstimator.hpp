#pragma once

#include "Time.hpp"
#include "UavSpecification.hpp"
#include "WindImpact.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <optional>
#include <vector>

namespace domain {

struct TestDurationEstimatorCalibration {
    double airDensity{1.225};
    double gravity{9.80665};
    double usableCapacityRatio{0.80};
    double defaultHoverToPeakRatio{0.45};
    double thrustPowerExponent{1.5};
    double minCos{0.2};
    double fallbackFrontalAreaM2{0.35};
    double fallbackDragCoefficient{1.1};
};

enum class TestDurationDiagnosticCode {
    TotalMassMissing,
    TotalMassEstimated,
    BatteryCapacityMissing,
    BatteryCellCountMissing,
    BatteryCellVoltageMissing,
    MotorCountMissing,
    MotorPeakCurrentMissing,
    FrontalAreaFallbackUsed,
    DragCoefficientFallbackUsed,
    MotorMaxThrustMissing,
    RequiredThrustExceedsAvailable,
    MotorPeakCurrentExceeded,
    BatteryDischargeCurrentExceeded,
    AngleOfAttackClampedByMinCos
};

struct TestDurationDiagnostic {
    TestDurationDiagnosticCode code{};
};

struct TestDurationDiagnosticValues {
    double windSpeedMps{0.0};
    double frontalAreaM2{0.0};
    double dragCoefficient{0.0};
    double dragForceN{0.0};
    double totalMassKg{0.0};
    double weightForceN{0.0};
    double anglePenalty{1.0};
    double requiredThrustN{0.0};
    double requiredThrustKg{0.0};
    double availableThrustKg{0.0};
    double batteryEnergyWh{0.0};
    double estimatedCurrentA{0.0};
    double maxMotorCurrentA{0.0};
    double maxBatteryCurrentA{0.0};
    double estimatedPowerW{0.0};
    double estimatedDurationMinutes{0.0};
};

struct EstimatedTestDurationResult {
    std::optional<DurationMinutes> duration{};
    std::vector<TestDurationDiagnostic> warnings{};
    std::vector<TestDurationDiagnostic> errors{};
    TestDurationDiagnosticValues values{};

    [[nodiscard]] bool hasErrors() const {
        return !errors.empty();
    }
};

struct TestDurationEstimationContext {
    UavSpecification uav{};
    WindImpact impact{};
    TestDurationEstimatorCalibration calibration{};
};

struct TestDurationOperationalLimitContext {
    const UavSpecification &uav;
    const TestDurationEstimatorCalibration &calibration;
    double requiredThrustN{0.0};
    double totalCurrentA{0.0};
};

class TestDurationEstimator final {
  public:
    [[nodiscard]] static EstimatedTestDurationResult estimate(const TestDurationEstimationContext &context) {
        EstimatedTestDurationResult result{};
        const auto &uav = context.uav;
        const auto &calibration = context.calibration;

        const auto totalMass = determineTotalMassKg(uav, result);
        validateCriticalInputs(uav, result);
        if (!totalMass.has_value() || result.hasErrors()) {
            return result;
        }

        const auto batteryCapacityAh = static_cast<double>(uav.battery.capacityMah) / 1000.0;
        const auto usableCapacityRatio = usableCapacityRatioFor(uav, calibration);
        const auto frontalArea = frontalAreaFor(uav, calibration, result);
        const auto dragCoefficient = dragCoefficientFor(uav, calibration, result);
        const auto windSpeed = beaufortToMetersPerSecond(context.impact.beaufort);
        const auto dragForce = 0.5 * calibration.airDensity * windSpeed * windSpeed * frontalArea * dragCoefficient;
        const auto weightForce = *totalMass * calibration.gravity;
        const auto anglePenalty = anglePenaltyFor(context.impact.angleOfAttack, calibration, result);
        const auto requiredThrust = std::hypot(weightForce, dragForce) * anglePenalty;
        const auto hoverCurrent = hoverCurrentFor(uav, calibration);
        const auto nominalHoverThrust = nominalHoverThrustFor(uav, calibration, weightForce, result);
        const auto loadRatio = requiredThrust / nominalHoverThrust;
        const auto currentPerMotor = hoverCurrent * std::pow(loadRatio, calibration.thrustPowerExponent);
        const auto totalCurrent = currentPerMotor * static_cast<double>(uav.motor.count) + uav.frame.equipmentCurrent;
        const auto batteryVoltage = static_cast<double>(uav.battery.cellCount) * uav.battery.cellVoltage;
        const auto estimatedPower = totalCurrent * batteryVoltage;
        const auto batteryEnergy = batteryCapacityAh * batteryVoltage * usableCapacityRatio;
        const auto estimatedDuration = estimatedPower > 0.0 ? batteryEnergy / estimatedPower * 60.0 : 0.0;

        result.values.windSpeedMps = windSpeed;
        result.values.frontalAreaM2 = frontalArea;
        result.values.dragCoefficient = dragCoefficient;
        result.values.dragForceN = dragForce;
        result.values.totalMassKg = *totalMass;
        result.values.weightForceN = weightForce;
        result.values.anglePenalty = anglePenalty;
        result.values.requiredThrustN = requiredThrust;
        result.values.requiredThrustKg = requiredThrust / calibration.gravity;
        result.values.batteryEnergyWh = batteryEnergy;
        result.values.estimatedCurrentA = totalCurrent;
        result.values.maxMotorCurrentA = uav.motor.peakCurrentA * static_cast<double>(uav.motor.count);
        result.values.maxBatteryCurrentA = batteryCapacityAh * uav.battery.dischargeRateC;
        result.values.estimatedPowerW = estimatedPower;
        result.values.estimatedDurationMinutes = estimatedDuration;

        validateOperationalLimits(
            TestDurationOperationalLimitContext{
                .uav = uav,
                .calibration = calibration,
                .requiredThrustN = requiredThrust,
                .totalCurrentA = totalCurrent,
            },
            result);

        if (!result.hasErrors() && estimatedDuration > 0.0) {
            result.duration = DurationMinutes::required(static_cast<int>(std::ceil(estimatedDuration)));
        }

        return result;
    }

    [[nodiscard]] static double batteryEnergyWh(const UavSpecification &uav,
                                                const TestDurationEstimatorCalibration &calibration = {}) {
        const auto batteryCapacityAh = static_cast<double>(uav.battery.capacityMah) / 1000.0;
        return batteryCapacityAh * static_cast<double>(uav.battery.cellCount) * uav.battery.cellVoltage *
               usableCapacityRatioFor(uav, calibration);
    }

    [[nodiscard]] static double beaufortToMetersPerSecond(Beaufort beaufort) {
        constexpr std::array<double, 8> beaufortSpeedMps{0.0, 1.0, 2.6, 4.4, 6.7, 9.4, 12.3, 15.5};
        const auto index = std::clamp(static_cast<int>(std::round(beaufort.value())), 0,
                                      static_cast<int>(beaufortSpeedMps.size() - 1));
        return beaufortSpeedMps[static_cast<std::size_t>(index)];
    }

  private:
    static void addWarning(EstimatedTestDurationResult &result, TestDurationDiagnosticCode code) {
        result.warnings.push_back(TestDurationDiagnostic{.code = code});
    }

    static void addError(EstimatedTestDurationResult &result, TestDurationDiagnosticCode code) {
        result.errors.push_back(TestDurationDiagnostic{.code = code});
    }

    [[nodiscard]] static double usableCapacityRatioFor(const UavSpecification &uav,
                                                       const TestDurationEstimatorCalibration &calibration) {
        if (uav.battery.usableCapacityPercent > 0.0) {
            return std::clamp(uav.battery.usableCapacityPercent / 100.0, 0.0, 1.0);
        }
        return calibration.usableCapacityRatio;
    }

    [[nodiscard]] static std::optional<double> determineTotalMassKg(const UavSpecification &uav,
                                                                    EstimatedTestDurationResult &result) {
        if (uav.frame.totalWeightKg > 0.0) {
            return uav.frame.totalWeightKg;
        }

        const auto estimatedMass =
            uav.frame.flightPayload + uav.battery.weight + uav.motor.weight * static_cast<double>(uav.motor.count);
        if (estimatedMass > 0.0) {
            addWarning(result, TestDurationDiagnosticCode::TotalMassEstimated);
            return estimatedMass;
        }

        addError(result, TestDurationDiagnosticCode::TotalMassMissing);
        return std::nullopt;
    }

    static void validateCriticalInputs(const UavSpecification &uav, EstimatedTestDurationResult &result) {
        if (uav.battery.capacityMah <= 0) {
            addError(result, TestDurationDiagnosticCode::BatteryCapacityMissing);
        }
        if (uav.battery.cellCount <= 0) {
            addError(result, TestDurationDiagnosticCode::BatteryCellCountMissing);
        }
        if (uav.battery.cellVoltage <= 0.0) {
            addError(result, TestDurationDiagnosticCode::BatteryCellVoltageMissing);
        }
        if (uav.motor.count <= 0) {
            addError(result, TestDurationDiagnosticCode::MotorCountMissing);
        }
        if (uav.motor.peakCurrentA <= 0.0) {
            addError(result, TestDurationDiagnosticCode::MotorPeakCurrentMissing);
        }
    }

    [[nodiscard]] static double frontalAreaFor(const UavSpecification &uav,
                                               const TestDurationEstimatorCalibration &calibration,
                                               EstimatedTestDurationResult &result) {
        if (uav.frame.frontalAreaM2 > 0.0) {
            return uav.frame.frontalAreaM2;
        }

        addWarning(result, TestDurationDiagnosticCode::FrontalAreaFallbackUsed);
        return calibration.fallbackFrontalAreaM2;
    }

    [[nodiscard]] static double dragCoefficientFor(const UavSpecification &uav,
                                                   const TestDurationEstimatorCalibration &calibration,
                                                   EstimatedTestDurationResult &result) {
        if (uav.frame.dragCoefficient > 0.0) {
            return uav.frame.dragCoefficient;
        }

        addWarning(result, TestDurationDiagnosticCode::DragCoefficientFallbackUsed);
        return calibration.fallbackDragCoefficient;
    }

    [[nodiscard]] static double anglePenaltyFor(AngleOfAttack angleOfAttack,
                                                const TestDurationEstimatorCalibration &calibration,
                                                EstimatedTestDurationResult &result) {
        constexpr double piValue = 3.14159265358979323846;
        const auto angleRad = std::abs(angleOfAttack.degrees()) * piValue / 180.0;
        const auto cosine = std::cos(angleRad);
        if (cosine < calibration.minCos) {
            addWarning(result, TestDurationDiagnosticCode::AngleOfAttackClampedByMinCos);
        }

        return 1.0 / std::max(cosine, calibration.minCos);
    }

    [[nodiscard]] static double hoverCurrentFor(const UavSpecification &uav,
                                                const TestDurationEstimatorCalibration &calibration) {
        if (uav.motor.hoverCurrentA > 0.0) {
            return uav.motor.hoverCurrentA;
        }
        if (uav.motor.nominalCurrentA > 0.0) {
            return uav.motor.nominalCurrentA;
        }
        return uav.motor.peakCurrentA * calibration.defaultHoverToPeakRatio;
    }

    [[nodiscard]] static double nominalHoverThrustFor(const UavSpecification &uav,
                                                      const TestDurationEstimatorCalibration &calibration,
                                                      double weightForceN, EstimatedTestDurationResult &result) {
        if (uav.motor.maxThrustKg > 0.0) {
            return uav.motor.maxThrustKg * static_cast<double>(uav.motor.count) * calibration.gravity;
        }

        addWarning(result, TestDurationDiagnosticCode::MotorMaxThrustMissing);
        return weightForceN;
    }

    static void validateOperationalLimits(const TestDurationOperationalLimitContext &context,
                                          EstimatedTestDurationResult &result) {
        if (context.uav.motor.maxThrustKg > 0.0) {
            const auto availableThrustKg = context.uav.motor.maxThrustKg * static_cast<double>(context.uav.motor.count);
            result.values.availableThrustKg = availableThrustKg;
            if (context.requiredThrustN > availableThrustKg * context.calibration.gravity) {
                addError(result, TestDurationDiagnosticCode::RequiredThrustExceedsAvailable);
            }
        }

        if (context.totalCurrentA > result.values.maxMotorCurrentA) {
            addWarning(result, TestDurationDiagnosticCode::MotorPeakCurrentExceeded);
        }

        if (context.uav.battery.dischargeRateC > 0.0 && context.totalCurrentA > result.values.maxBatteryCurrentA) {
            addWarning(result, TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded);
        }
    }
};

} // namespace domain
