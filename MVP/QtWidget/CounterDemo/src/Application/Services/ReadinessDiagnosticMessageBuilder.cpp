#include "ReadinessDiagnosticMessageBuilder.hpp"

#include "../../Localization/ReadinessStrings.hpp"

#include <iomanip>
#include <sstream>

namespace application::services {
namespace {

std::string number(double value, int precision = 2) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(precision) << value;
    return out.str();
}

std::string summaryFor(application::session::ReadinessStatus status) {
    switch (status) {
    case application::session::ReadinessStatus::Ok:
        return localization::readiness::summaryOk;
    case application::session::ReadinessStatus::Warning:
        return localization::readiness::summaryWarning;
    case application::session::ReadinessStatus::Dangerous:
        return localization::readiness::summaryDangerous;
    case application::session::ReadinessStatus::Failed:
        return localization::readiness::summaryFailed;
    case application::session::ReadinessStatus::Unknown:
        return localization::readiness::summaryUnknown;
    }

    return localization::readiness::summaryUnknown;
}

} // namespace

std::string ReadinessDiagnosticMessage::toDisplayText() const {
    if (details.empty()) {
        return summary;
    }

    std::string text = summary;
    for (const auto &detail : details) {
        text += "\n- " + detail;
    }
    return text;
}

ReadinessDiagnosticMessage ReadinessDiagnosticMessageBuilder::build(const session::ReadinessStateData &readiness) {
    ReadinessDiagnosticMessage message{.summary = summaryFor(readiness.status)};

    if (readiness.calculatedForWorstCaseScenario && readiness.hasCalculatedForImpact) {
        message.details.push_back(localization::readiness::worstCaseScenarioImpact(
            number(readiness.calculatedForImpact.beaufort.value(), 1),
            number(readiness.calculatedForImpact.angleOfAttack.degrees(), 1)));
    }

    for (const auto &error : readiness.errors) {
        message.details.push_back(messageForDiagnostic(error.code, readiness.values));
    }

    for (const auto &warning : readiness.warnings) {
        message.details.push_back(messageForDiagnostic(warning.code, readiness.values));
    }

    if (readiness.safeLimits.status == domain::SafeWindImpactLimitStatus::Available) {
        message.details.push_back(
            localization::readiness::safeBeaufortLimit(number(readiness.safeLimits.maxSafeBeaufort, 1)));
        message.details.push_back(
            localization::readiness::safeAngleOfAttackLimit(number(readiness.safeLimits.maxSafeAbsAngleOfAttack, 0)));
    } else if (!readiness.safeLimits.diagnostics.empty()) {
        message.details.push_back(localization::readiness::safeLimitsUnavailable);
    }

    return message;
}

std::string
ReadinessDiagnosticMessageBuilder::messageForDiagnostic(domain::TestDurationDiagnosticCode code,
                                                        const domain::TestDurationDiagnosticValues &values) {
    switch (code) {
    case domain::TestDurationDiagnosticCode::TotalMassMissing:
        return localization::readiness::totalMassMissing;
    case domain::TestDurationDiagnosticCode::TotalMassEstimated:
        return localization::readiness::totalMassEstimated(number(values.totalMassKg));
    case domain::TestDurationDiagnosticCode::BatteryCapacityMissing:
        return localization::readiness::batteryCapacityMissing;
    case domain::TestDurationDiagnosticCode::BatteryCellCountMissing:
        return localization::readiness::batteryCellCountMissing;
    case domain::TestDurationDiagnosticCode::BatteryCellVoltageMissing:
        return localization::readiness::batteryCellVoltageMissing;
    case domain::TestDurationDiagnosticCode::MotorCountMissing:
        return localization::readiness::motorCountMissing;
    case domain::TestDurationDiagnosticCode::MotorPeakCurrentMissing:
        return localization::readiness::motorPeakCurrentMissing;
    case domain::TestDurationDiagnosticCode::FrontalAreaFallbackUsed:
        return localization::readiness::frontalAreaFallbackUsed(number(values.frontalAreaM2));
    case domain::TestDurationDiagnosticCode::DragCoefficientFallbackUsed:
        return localization::readiness::dragCoefficientFallbackUsed(number(values.dragCoefficient));
    case domain::TestDurationDiagnosticCode::MotorMaxThrustMissing:
        return localization::readiness::motorMaxThrustMissing;
    case domain::TestDurationDiagnosticCode::RequiredThrustExceedsAvailable:
        return localization::readiness::requiredThrustExceedsAvailable(number(values.requiredThrustKg),
                                                                       number(values.availableThrustKg));
    case domain::TestDurationDiagnosticCode::MotorPeakCurrentExceeded:
        return localization::readiness::motorPeakCurrentExceeded(number(values.estimatedCurrentA),
                                                                 number(values.maxMotorCurrentA));
    case domain::TestDurationDiagnosticCode::BatteryDischargeCurrentExceeded:
        return localization::readiness::batteryDischargeCurrentExceeded(number(values.estimatedCurrentA),
                                                                        number(values.maxBatteryCurrentA));
    case domain::TestDurationDiagnosticCode::AngleOfAttackClampedByMinCos:
        return localization::readiness::angleOfAttackClampedByMinCos;
    }

    return localization::readiness::unknownDiagnostic;
}

} // namespace application::services
