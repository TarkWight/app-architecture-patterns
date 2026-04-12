cmake_minimum_required(VERSION 4.2.1)
project("MVP - Counter" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
option(ENABLE_CLANG_FORMAT_CHECK "Enable clang-format check" ON)

# in presets:
set(CLANG_TIDY_PATH "" CACHE FILEPATH "Path to clang-tidy executable")
set(CLANG_FORMAT_PATH "" CACHE FILEPATH "Path to clang-format executable")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

qt_add_executable(develop
  src/main.cpp

  src/Domain/Plot.hpp
  src/Domain/TestProtocol.hpp
  src/Domain/Time.hpp
  src/Domain/FormulaExpression.hpp
  src/Domain/WindProfile.hpp
  src/Domain/AxisState.hpp


  src/Application/Dto/PdfDocumentModel.hpp

  src/Application/Ports/ILogger.hpp
  src/Application/Ports/IFunctionEngine.hpp
  src/Application/Ports/ITimerService.hpp
  src/Application/Ports/IPdfExporter.hpp

  src/Application/Session/SessionState.hpp
  src/Application/Session/SessionState.cpp
  src/Application/Session/SessionStateData.hpp
  src/Application/Session/Subscription.hpp
  src/Application/Session/Subscription.cpp


  src/Application/UseCases/BuildControlPlotUseCase.hpp
  src/Application/UseCases/BuildControlPlotUseCase.cpp
  src/Application/UseCases/GenerateStairPlotUseCase.hpp
  src/Application/UseCases/GenerateStairPlotUseCase.cpp
  src/Application/UseCases/StartTimerUseCase.hpp
  src/Application/UseCases/StartTimerUseCase.cpp
  src/Application/UseCases/StopTimerUseCase.hpp
  src/Application/UseCases/StopTimerUseCase.cpp
  src/Application/UseCases/ExportPdfUseCase.hpp
  src/Application/UseCases/ExportPdfUseCase.cpp
  src/Application/UseCases/SetFunctionExpressionUseCase.hpp
  src/Application/UseCases/SetFunctionExpressionUseCase.cpp
  src/Application/UseCases/SetLineColorUseCase.hpp
  src/Application/UseCases/SetLineColorUseCase.cpp
  src/Application/UseCases/SetTimerDurationUseCase.hpp
  src/Application/UseCases/SetTimerDurationUseCase.cpp
  src/Application/UseCases/UpdateTestProtocolUseCase.hpp
  src/Application/UseCases/UpdateTestProtocolUseCase.cpp
  src/Application/UseCases/SetControlChartsTabMinutesUseCase.hpp
  src/Application/UseCases/SetControlChartsTabMinutesUseCase.cpp
  src/Application/UseCases/SetWindProfileUseCase.hpp
  src/Application/UseCases/SetWindProfileUseCase.cpp
  src/Application/ApplicationContext.hpp

  src/Infrastructure/QtTextLogger.hpp
  src/Infrastructure/QtTextLogger.cpp
  src/Infrastructure/SimpleFunctionEngineStub.hpp
  src/Infrastructure/QtTimerService.hpp
  src/Infrastructure/QtTimerService.cpp
  src/Infrastructure/SessionStateQtAdapter.hpp
  src/Infrastructure/SessionStateQtAdapter.cpp
  src/Infrastructure/QtPdfExporter.hpp
  src/Infrastructure/QtPdfExporter.cpp

  src/Presentation/IShellView.hpp
  src/Presentation/TelemetryChartsTab/ITelemetryChartsTabView.hpp
  src/Presentation/ControlChartsTab/IControlChartsTabView.hpp
  src/Presentation/TestProtocolTab/ITestProtocolTabView.hpp
  src/Presentation/ShellPresenter.hpp
  src/Presentation/ShellPresenter.cpp
  src/Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.hpp
  src/Presentation/TelemetryChartsTab/TelemetryChartsTabPresenter.cpp
  src/Presentation/ControlChartsTab/ControlChartsTabPresenter.hpp
  src/Presentation/ControlChartsTab/ControlChartsTabPresenter.cpp
  src/Presentation/TestProtocolTab/TestProtocolTabPresenter.hpp
  src/Presentation/TestProtocolTab/TestProtocolTabPresenter.cpp

  src/Ui/MainWindowUiAdapter.hpp
  src/Ui/MainWindowUiAdapter.cpp
  src/Ui/TelemetryChartsTabWidget.hpp
  src/Ui/TelemetryChartsTabWidget.cpp
  src/Ui/ControlChartsTabWidget.hpp
  src/Ui/ControlChartsTabWidget.cpp
  src/Ui/TestProtocolTabWidget.hpp
  src/Ui/TestProtocolTabWidget.cpp
  src/Ui/MainWindow.hpp
  src/Ui/MainWindow.cpp
  src/Ui/MainWindow.ui
  src/Ui/TelemetryChartsTabWidget.ui
  src/Ui/ControlChartsTabWidget.ui
  src/Ui/TestProtocolTabWidget.ui
  src/Ui/PlotWidget.hpp
  src/Ui/PlotWidget.cpp
  src/Ui/Render/PlotRenderer.hpp
  src/Ui/Render/PlotRenderer.cpp
  src/Domain/TestExecutionStatus.hpp
  src/Domain/TelemetryStatus.hpp

)
target_link_libraries(develop PRIVATE Qt::Core Qt::Widgets)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CodeQuality)
enable_code_quality(develop)

if (APPLE)
  set_target_properties(develop PROPERTIES MACOSX_BUNDLE TRUE)
endif()
