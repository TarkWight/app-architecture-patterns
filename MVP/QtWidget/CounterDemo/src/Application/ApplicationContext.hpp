#ifndef APPLICATIONCONTEXT_HPP
#define APPLICATIONCONTEXT_HPP

#include "Session/SessionState.hpp"

#include "UseCases/BuildFormulaPlotUseCase.hpp"
#include "UseCases/ExecuteCounterCommandUseCase.hpp"
#include "UseCases/ExportPdfUseCase.hpp"
#include "UseCases/GenerateStairPlotUseCase.hpp"
#include "UseCases/SetFunctionExpressionUseCase.hpp"
#include "UseCases/SetLineColorUseCase.hpp"
#include "UseCases/SetTab2MinutesUseCase.hpp"
#include "UseCases/SetTimerDurationUseCase.hpp"
#include "UseCases/StartTimerUseCase.hpp"
#include "UseCases/StopTimerUseCase.hpp"
#include "UseCases/UpdatePoemUseCase.hpp"

#include "../Infrastructure/InMemoryCounterRepository.hpp"
#include "../Infrastructure/QtPdfExporter.hpp"
#include "../Infrastructure/QtTextLogger.hpp"
#include "../Infrastructure/QtTimerService.hpp"
#include "../Infrastructure/SessionStateQtAdapter.hpp"
#include "../Infrastructure/SimpleFunctionEngineStub.hpp"

#include "../Presentation/ShellPresenter.hpp"
#include "../Presentation/Tab1/Tab1Presenter.hpp"
#include "../Presentation/Tab2/Tab2Presenter.hpp"
#include "../Presentation/Tab3/Tab3Presenter.hpp"

struct ApplicationContext {
    application::session::SessionState sessionState;

    infrastructure::InMemoryCounterRepository repository;

    infrastructure::QtTextLogger logger{[](const std::string &) {}};

    infrastructure::SimpleFunctionEngineStub functionEngine;
    infrastructure::QtTimerService timerService;
    infrastructure::QtPdfExporter pdfExporter;
    infrastructure::SessionStateQtAdapter sessionAdapter{sessionState};

    application::useCases::ExecuteCounterCommandUseCase executeCounterCommandUseCase{repository, logger};

    application::useCases::GenerateStairPlotUseCase generateStairPlotUseCase{sessionState};

    application::useCases::BuildFormulaPlotUseCase buildFormulaPlotUseCase{sessionState, functionEngine};

    application::useCases::StartTimerUseCase startTimerUseCase{sessionState, timerService};

    application::useCases::StopTimerUseCase stopTimerUseCase{sessionState, timerService};

    application::useCases::SetFunctionExpressionUseCase setFunctionExpressionUseCase{sessionState};

    application::useCases::SetLineColorUseCase setLineColorUseCase{sessionState};

    application::useCases::SetTab2MinutesUseCase setTab2MinutesUseCase{sessionState};

    application::useCases::SetTimerDurationUseCase setTimerDurationUseCase{sessionState};

    application::useCases::UpdatePoemUseCase updatePoemUseCase{sessionState};

    application::useCases::ExportPdfUseCase exportPdfUseCase{sessionState, pdfExporter};

    presentation::ShellPresenter shellPresenter{
        presentation::ShellPresenter::Dependencies{.state = sessionState,
                                                   .startTimerUseCase = startTimerUseCase,
                                                   .stopTimerUseCase = stopTimerUseCase,
                                                   .setFunctionExpressionUseCase = setFunctionExpressionUseCase,
                                                   .setLineColorUseCase = setLineColorUseCase,
                                                   .buildFormulaPlotUseCase = buildFormulaPlotUseCase}};

    presentation::tab1::Tab1Presenter tab1Presenter{executeCounterCommandUseCase, generateStairPlotUseCase};

    presentation::tab2::Tab2Presenter tab2Presenter{
        presentation::tab2::Tab2Presenter::Dependencies{.state = sessionState,
                                                        .executeCounterCommandUseCase = executeCounterCommandUseCase,
                                                        .setTab2MinutesUseCase = setTab2MinutesUseCase,
                                                        .buildFormulaPlotUseCase = buildFormulaPlotUseCase}};

    presentation::tab3::Tab3Presenter tab3Presenter{
        presentation::tab3::Tab3Presenter::Dependencies{.state = sessionState,
                                                        .executeCounterCommandUseCase = executeCounterCommandUseCase,
                                                        .setTimerDurationUseCase = setTimerDurationUseCase,
                                                        .updatePoemUseCase = updatePoemUseCase,
                                                        .exportPdfUseCase = exportPdfUseCase}};
};

#endif // APPLICATIONCONTEXT_HPP
