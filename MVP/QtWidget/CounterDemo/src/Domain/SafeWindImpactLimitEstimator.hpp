#ifndef SAFEWINDIMPACTLIMITESTIMATOR_HPP
#define SAFEWINDIMPACTLIMITESTIMATOR_HPP

#include "TestDurationEstimator.hpp"
#include "UavSpecification.hpp"
#include "WindImpact.hpp"

#include <vector>

namespace domain {

enum class SafeWindImpactLimitStatus { Available, Unavailable };

struct SafeWindImpactLimitResult {
    SafeWindImpactLimitStatus status{SafeWindImpactLimitStatus::Unavailable};
    double maxSafeBeaufort{0.0};
    double maxSafeAbsAngleOfAttack{0.0};
    std::vector<TestDurationDiagnostic> diagnostics{};
};

class SafeWindImpactLimitEstimator final {
  public:
    [[nodiscard]] static SafeWindImpactLimitResult estimate(const UavSpecification &uav, const WindImpact &baseImpact,
                                                            const TestDurationEstimatorCalibration &calibration = {}) {
        SafeWindImpactLimitResult result{};
        result.maxSafeBeaufort = maxSafeBeaufort(uav, baseImpact, calibration, result.diagnostics);
        result.maxSafeAbsAngleOfAttack = maxSafeAbsAngleOfAttack(uav, baseImpact, calibration, result.diagnostics);

        const auto baseline = TestDurationEstimator::estimate(TestDurationEstimationContext{
            .uav = uav,
            .impact = baseImpact,
            .calibration = calibration,
        });
        appendDiagnostics(result.diagnostics, baseline.warnings);
        appendDiagnostics(result.diagnostics, baseline.errors);

        if (!hasUnavailableDiagnostics(baseline.errors)) {
            result.status = SafeWindImpactLimitStatus::Available;
        }

        return result;
    }

  private:
    static void appendDiagnostics(std::vector<TestDurationDiagnostic> &target,
                                  const std::vector<TestDurationDiagnostic> &source) {
        target.insert(target.end(), source.begin(), source.end());
    }

    [[nodiscard]] static bool hasUnavailableDiagnostics(const std::vector<TestDurationDiagnostic> &diagnostics) {
        for (const auto &diagnostic : diagnostics) {
            switch (diagnostic.code) {
            case TestDurationDiagnosticCode::TotalMassMissing:
            case TestDurationDiagnosticCode::BatteryCapacityMissing:
            case TestDurationDiagnosticCode::BatteryCellCountMissing:
            case TestDurationDiagnosticCode::BatteryCellVoltageMissing:
            case TestDurationDiagnosticCode::MotorCountMissing:
            case TestDurationDiagnosticCode::MotorPeakCurrentMissing:
                return true;
            case TestDurationDiagnosticCode::TotalMassEstimated:
            case TestDurationDiagnosticCode::FrontalAreaFallbackUsed:
            case TestDurationDiagnosticCode::DragCoefficientFallbackUsed:
            case TestDurationDiagnosticCode::MotorMaxThrustMissing:
            case TestDurationDiagnosticCode::RequiredThrustExceedsAvailable:
            case TestDurationDiagnosticCode::MotorPeakCurrentExceeded:
            case TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded:
            case TestDurationDiagnosticCode::AngleOfAttackClampedByMinCos:
                break;
            }
        }

        return false;
    }

    [[nodiscard]] static double maxSafeBeaufort(const UavSpecification &uav, const WindImpact &baseImpact,
                                                const TestDurationEstimatorCalibration &calibration,
                                                std::vector<TestDurationDiagnostic> &diagnostics) {
        double maxSafe = 0.0;
        bool foundSafe = false;

        constexpr int maxBeaufortSteps = 16;
        constexpr double beaufortStep = 0.5;
        for (int step = 0; step <= maxBeaufortSteps; ++step) {
            const auto beaufort = static_cast<double>(step) * beaufortStep;
            const auto impact = baseImpact.withBeaufort(Beaufort::from(beaufort));
            const auto estimate = TestDurationEstimator::estimate(TestDurationEstimationContext{
                .uav = uav,
                .impact = impact,
                .calibration = calibration,
            });

            if (estimate.errors.empty()) {
                foundSafe = true;
                maxSafe = impact.beaufort.value();
            } else if (!foundSafe) {
                appendDiagnostics(diagnostics, estimate.errors);
            }
        }

        return maxSafe;
    }

    [[nodiscard]] static double maxSafeAbsAngleOfAttack(const UavSpecification &uav, const WindImpact &baseImpact,
                                                        const TestDurationEstimatorCalibration &calibration,
                                                        std::vector<TestDurationDiagnostic> &diagnostics) {
        double maxSafe = 0.0;
        bool foundSafe = false;

        constexpr int maxAngleSteps = 18;
        constexpr double angleStepDegrees = 5.0;
        for (int step = 0; step <= maxAngleSteps; ++step) {
            const auto angle = static_cast<double>(step) * angleStepDegrees;
            const auto impact = baseImpact.withAngleOfAttack(AngleOfAttack::from(angle));
            const auto estimate = TestDurationEstimator::estimate(TestDurationEstimationContext{
                .uav = uav,
                .impact = impact,
                .calibration = calibration,
            });

            if (estimate.errors.empty()) {
                foundSafe = true;
                maxSafe = angle;
            } else if (!foundSafe) {
                appendDiagnostics(diagnostics, estimate.errors);
            }
        }

        return maxSafe;
    }
};

} // namespace domain

#endif // SAFEWINDIMPACTLIMITESTIMATOR_HPP
