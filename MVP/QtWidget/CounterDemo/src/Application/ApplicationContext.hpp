#ifndef APPLICATIONCONTEXT_HPP
#define APPLICATIONCONTEXT_HPP

#include "Session/SessionState.hpp"

#include "UseCases/BuildControlPlotUseCase.hpp"
#include "UseCases/ExportPdfUseCase.hpp"
#include "UseCases/GenerateStairPlotUseCase.hpp"
#include "UseCases/PauseTestExecutionUseCase.hpp"
#include "UseCases/ResumeTestExecutionUseCase.hpp"
#include "UseCases/SetFunctionExpressionUseCase.hpp"
#include "UseCases/SetLineColorUseCase.hpp"
#include "UseCases/SetControlChartsTabMinutesUseCase.hpp"
#include "UseCases/SetTimerDurationUseCase.hpp"
#include "UseCases/StartTestExecutionUseCase.hpp"
#include "UseCases/StopTestExecutionUseCase.hpp"
#include "UseCases/UpdateTestProtocolUseCase.hpp"
#include "UseCases/SetWindProfileUseCase.hpp"

#include "../Infrastructure/QtPdfExporter.hpp"
#include "../Infrastructure/QtTextLogger.hpp"
#include "../Infrastructure/QtTestExecutionScheduler.hpp"
#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Infrastructure/SimpleFunctionEngineStub.hpp"

#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp"
#include "../Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp"
#include "../Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp"

struct ApplicationContext {
    application::session::SessionState sessionState;

    infrastructure::QtTextLogger logger{[](const std::string &) {}};

    infrastructure::SimpleFunctionEngineStub functionEngine;
    infrastructure::QtTestExecutionScheduler testExecutionScheduler;
    infrastructure::QtPdfExporter pdfExporter;
    infrastructure::SessionStateQtAdapter sessionAdapter{sessionState};

    application::useCases::GenerateStairPlotUseCase generateStairPlotUseCase{sessionState};

    application::useCases::BuildControlPlotUseCase buildControlPlotUseCase{sessionState, functionEngine};

    application::useCases::StartTestExecutionUseCase startTestExecutionUseCase{
                                                                               sessionState, testExecutionScheduler};

    application::useCases::PauseTestExecutionUseCase pauseTestExecutionUseCase{
                                                                               sessionState, testExecutionScheduler};

    application::useCases::ResumeTestExecutionUseCase resumeTestExecutionUseCase{
                                                                                 sessionState, testExecutionScheduler};

    application::useCases::StopTestExecutionUseCase stopTestExecutionUseCase{
                                                                             sessionState, testExecutionScheduler};

    application::useCases::SetFunctionExpressionUseCase setFunctionExpressionUseCase{sessionState};

    application::useCases::SetLineColorUseCase setLineColorUseCase{sessionState};

    application::useCases::SetControlChartsTabMinutesUseCase setControlChartsTabMinutesUseCase{sessionState};

    application::useCases::SetWindProfileUseCase setWindProfileUseCase{sessionState};

    application::useCases::SetTimerDurationUseCase setTimerDurationUseCase{sessionState};

    application::useCases::UpdateTestProtocolUseCase updateTestProtocolUseCase{sessionState};

    application::useCases::ExportPdfUseCase exportPdfUseCase{sessionState, pdfExporter};

    presentation::ShellPresenter shellPresenter{
        presentation::ShellPresenter::Dependencies{
            .state = sessionState,
            .startTestExecutionUseCase = startTestExecutionUseCase,
            .pauseTestExecutionUseCase = pauseTestExecutionUseCase,
            .resumeTestExecutionUseCase = resumeTestExecutionUseCase,
            .stopTestExecutionUseCase = stopTestExecutionUseCase,
            .setFunctionExpressionUseCase = setFunctionExpressionUseCase,
            .setLineColorUseCase = setLineColorUseCase,
            .buildControlPlotUseCase = buildControlPlotUseCase
        }
    };

    presentation::telemetryChartsTab::TelemetryChartsTabPresenter telemetryChartsTabPresenter{
                                                                                              generateStairPlotUseCase};

    presentation::controlChartsTab::ControlChartsTabPresenter controlChartsTabPresenter{
        presentation::controlChartsTab::ControlChartsTabPresenter::Dependencies{
            .state = sessionState,
            .setControlChartsTabMinutesUseCase = setControlChartsTabMinutesUseCase,
            .setWindProfileUseCase = setWindProfileUseCase,
            .buildControlPlotUseCase = buildControlPlotUseCase
        }
    };

    presentation::testProtocolTab::TestProtocolTabPresenter testProtocolTabPresenter{
        presentation::testProtocolTab::TestProtocolTabPresenter::Dependencies{
            .state = sessionState,
            .setTimerDurationUseCase = setTimerDurationUseCase,
            .updateTestProtocolUseCase = updateTestProtocolUseCase,
            .exportPdfUseCase = exportPdfUseCase
        }
    };
};

#endif // APPLICATIONCONTEXT_HPP