#ifndef APPLICATIONCONTEXT_HPP
#define APPLICATIONCONTEXT_HPP

#include "Session/SessionState.hpp"

#include "UseCases/BuildControlPlotUseCase.hpp"
#include "UseCases/ExportPdfUseCase.hpp"
#include "UseCases/GenerateStairPlotUseCase.hpp"
#include "UseCases/LoadPdfReportDefaultsUseCase.hpp"
#include "UseCases/PauseTestExecutionUseCase.hpp"
#include "UseCases/ResumeTestExecutionUseCase.hpp"
#include "UseCases/SetFunctionExpressionUseCase.hpp"
#include "UseCases/SetLineColorUseCase.hpp"
#include "UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "UseCases/SetOperatorTestDurationUseCase.hpp"
#include "UseCases/SetTestTimeSourceUseCase.hpp"
#include "UseCases/SetTelemetryAxisColorUseCase.hpp"
#include "UseCases/SetTelemetryAxisVisibleUseCase.hpp"
#include "UseCases/SetTelemetryWindowUseCase.hpp"
#include "UseCases/SetStandControlModeUseCase.hpp"
#include "UseCases/SetStandImpactUseCase.hpp"
#include "UseCases/ApplyBeaufortImpactUseCase.hpp"
#include "UseCases/ApplyWindDirectionUseCase.hpp"
#include "UseCases/ApplyAngleOfAttackUseCase.hpp"
#include "UseCases/StartTestExecutionUseCase.hpp"
#include "UseCases/StopTestExecutionUseCase.hpp"
#include "UseCases/UpdateTestProtocolUseCase.hpp"
#include "UseCases/SetWindImpactUseCase.hpp"
#include "UseCases/ConfigureTelemetryUseCase.hpp"
#include "UseCases/ConnectStandUseCase.hpp"
#include "UseCases/DisconnectStandUseCase.hpp"
#include "Services/TelemetrySessionClock.hpp"

#include "../Infrastructure/Config/TomlConfigRepository.hpp"
#include "../Infrastructure/ConfigTemplates/ConfigTemplateService.hpp"
#include "../Infrastructure/ConfigTemplates/QtAppFileLocationProvider.hpp"

#include "../Infrastructure/AxisTcp/LegacyAxisProtocolCodec.hpp"
#include "../Infrastructure/AxisTcp/QtTcpTelemetryClient.hpp"

#include "../Infrastructure/QtPdfExporter.hpp"
#include "../Infrastructure/QtTextLogger.hpp"
#include "../Infrastructure/QtTestExecutionScheduler.hpp"
#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Infrastructure/SimpleFunctionEngine.hpp"

#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp"
#include "../Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"

#include <string>

struct ApplicationContext {
    infrastructure::configTemplates::QtAppFileLocationProvider appFileLocationProvider;
    infrastructure::configTemplates::ConfigTemplateService configTemplateService{appFileLocationProvider};
    std::string pdfReportConfigPath{
        configTemplateService.pathFor(infrastructure::configTemplates::ConfigTemplateType::PdfReport).string()};

    application::session::SessionState sessionState;

    infrastructure::QtTextLogger logger{[](const std::string &) {}};

    infrastructure::SimpleFunctionEngine functionEngine;
    infrastructure::QtTestExecutionScheduler testExecutionScheduler;
    infrastructure::QtPdfExporter pdfExporter;
    infrastructure::SessionStateQtAdapter sessionAdapter{sessionState};
    infrastructure::config::TomlConfigRepository configRepository;
    application::services::TelemetrySessionClock telemetrySessionClock;

    infrastructure::axisTcp::LegacyAxisProtocolCodec axisProtocolCodec;

    infrastructure::axisTcp::QtTcpTelemetryClient telemetryClient{axisProtocolCodec};

    application::useCases::ConfigureTelemetryUseCase configureTelemetryUseCase{sessionState, configRepository,
                                                                               telemetryClient, telemetrySessionClock};

    application::useCases::ConnectStandUseCase connectStandUseCase{sessionState, telemetryClient};

    application::useCases::DisconnectStandUseCase disconnectStandUseCase{sessionState, telemetryClient};

    application::useCases::GenerateStairPlotUseCase generateStairPlotUseCase{sessionState};

    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{sessionState, functionEngine};

    application::useCases::EstimateTestDurationUseCase estimateTestDurationUseCase{sessionState};

    application::useCases::StartTestExecutionUseCase startTestExecutionUseCase{
        application::useCases::StartTestExecutionUseCase::Dependencies{.state = sessionState,
                                                                       .testExecutionScheduler = testExecutionScheduler,
                                                                       .telemetryClient = telemetryClient,
                                                                       .buildControlPlotUseCase =
                                                                           buildControlPlotUseCase,
                                                                       .telemetrySessionClock = telemetrySessionClock}};

    application::useCases::PauseTestExecutionUseCase pauseTestExecutionUseCase{sessionState, testExecutionScheduler,
                                                                               telemetryClient, telemetrySessionClock};

    application::useCases::ResumeTestExecutionUseCase resumeTestExecutionUseCase{
        sessionState, testExecutionScheduler, telemetryClient, telemetrySessionClock};

    application::useCases::StopTestExecutionUseCase stopTestExecutionUseCase{sessionState, testExecutionScheduler,
                                                                             telemetryClient, telemetrySessionClock};

    application::useCases::SetTestTimeSourceUseCase setTestTimeSourceUseCase{sessionState};

    application::useCases::SetTelemetryWindowUseCase setTelemetryWindowUseCase{sessionState};

    application::useCases::SetTelemetryAxisColorUseCase setTelemetryAxisColorUseCase{sessionState};

    application::useCases::SetTelemetryAxisVisibleUseCase setTelemetryAxisVisibleUseCase{sessionState};

    application::useCases::SetStandControlModeUseCase setStandControlModeUseCase{sessionState};

    application::useCases::SetStandImpactUseCase setStandImpactUseCase{sessionState, telemetryClient};

    application::useCases::ApplyBeaufortImpactUseCase applyBeaufortImpactUseCase{sessionState};

    application::useCases::ApplyWindDirectionUseCase applyWindDirectionUseCase{sessionState};

    application::useCases::ApplyAngleOfAttackUseCase applyAngleOfAttackUseCase{sessionState};

    application::useCases::SetFunctionExpressionUseCase setFunctionExpressionUseCase{sessionState};

    application::useCases::SetLineColorUseCase setLineColorUseCase{sessionState};

    application::useCases::SetControlChartsTabMinutesUseCase setControlChartsTabMinutesUseCase{sessionState};

    application::useCases::SetWindImpactUseCase setWindImpactUseCase{sessionState};

    application::useCases::SetOperatorTestDurationUseCase setOperatorTestDurationUseCase{sessionState};

    application::useCases::UpdateTestProtocolUseCase updateTestProtocolUseCase{sessionState};

    application::useCases::LoadPdfReportDefaultsUseCase loadPdfReportDefaultsUseCase{sessionState, configRepository};

    application::useCases::ExportPdfUseCase exportPdfUseCase{sessionState, pdfExporter};

    presentation::ShellPresenter shellPresenter{
        presentation::ShellPresenter::Dependencies{.state = sessionState,
                                                   .startTestExecutionUseCase = startTestExecutionUseCase,
                                                   .pauseTestExecutionUseCase = pauseTestExecutionUseCase,
                                                   .resumeTestExecutionUseCase = resumeTestExecutionUseCase,
                                                   .stopTestExecutionUseCase = stopTestExecutionUseCase,
                                                   .setTestTimeSourceUseCase = setTestTimeSourceUseCase,
                                                   .setFunctionExpressionUseCase = setFunctionExpressionUseCase,
                                                   .setLineColorUseCase = setLineColorUseCase,
                                                   .setStandControlModeUseCase = setStandControlModeUseCase,
                                                   .buildControlPlotUseCase = buildControlPlotUseCase,
                                                   .estimateTestDurationUseCase = estimateTestDurationUseCase,
                                                   .configureTelemetryUseCase = configureTelemetryUseCase,
                                                   .connectStandUseCase = connectStandUseCase,
                                                   .disconnectStandUseCase = disconnectStandUseCase}};

    presentation::telemetryChartsTab::TelemetryChartsTabPresenter telemetryChartsTabPresenter{
        presentation::telemetryChartsTab::TelemetryChartsTabPresenter::Dependencies{
            .setTelemetryWindowUseCase = setTelemetryWindowUseCase,
            .setTelemetryAxisColorUseCase = setTelemetryAxisColorUseCase,
            .setTelemetryAxisVisibleUseCase = setTelemetryAxisVisibleUseCase}};

    presentation::controlChartsTab::ControlChartsTabPresenter controlChartsTabPresenter{
        presentation::controlChartsTab::ControlChartsTabPresenter::Dependencies{
            .state = sessionState,
            .setControlChartsTabMinutesUseCase = setControlChartsTabMinutesUseCase,
            .setWindImpactUseCase = setWindImpactUseCase,
            .buildControlPlotUseCase = buildControlPlotUseCase,
            .estimateTestDurationUseCase = estimateTestDurationUseCase}};

    presentation::testProtocolTab::TestProtocolTabPresenter testProtocolTabPresenter{
        presentation::testProtocolTab::TestProtocolTabPresenter::Dependencies{
            .state = sessionState,
            .setOperatorTestDurationUseCase = setOperatorTestDurationUseCase,
            .updateTestProtocolUseCase = updateTestProtocolUseCase,
            .loadPdfReportDefaultsUseCase = loadPdfReportDefaultsUseCase,
            .exportPdfUseCase = exportPdfUseCase,
            .pdfReportConfigPath = pdfReportConfigPath}};
};

#endif // APPLICATIONCONTEXT_HPP
