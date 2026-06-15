cmake_minimum_required(VERSION 4.0.1)
project("UAS Test Stand MVP" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_CLANG_TIDY "Enable clang-tidy" ON)
option(ENABLE_CLANG_FORMAT_CHECK "Enable clang-format check" ON)

# in presets:
set(CLANG_TIDY_PATH "" CACHE FILEPATH "Path to clang-tidy executable")
set(CLANG_FORMAT_PATH "" CACHE FILEPATH "Path to clang-format executable")

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)
qt_standard_project_setup()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

include(FetchContent)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(tomlplusplus)

include(CTest)
if (BUILD_TESTING)
  FetchContent_Declare(
      googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG v1.15.2
  )
  FetchContent_MakeAvailable(googletest)
endif()

qt_add_executable(develop
  src/main.cpp

  src/Domain/Plot.hpp
  src/Domain/FormulaTemplate.hpp
  src/Domain/ControlTrace.hpp
  src/Domain/ControlProfileTiming.hpp
  src/Domain/TestProtocol.hpp
  src/Domain/Time.hpp
  src/Domain/Expression.hpp
  src/Domain/WindImpact.hpp
  src/Domain/WindControlProfile.hpp
  src/Domain/WindControlProfileImpact.hpp
  src/Domain/AxisState.hpp
  src/Domain/TestExecutionStatus.hpp
  src/Domain/TestExecutionTransitions.hpp
  src/Domain/TelemetryStatus.hpp
  src/Domain/StandConnectionStatus.hpp
  src/Domain/StandImpactTransition.hpp
  src/Domain/StandScenario.hpp
  src/Domain/TestTimeSource.hpp
  src/Domain/TestTimeDirection.hpp
  src/Domain/Id.hpp
  src/Domain/AxisId.hpp
  src/Domain/AxisTelemetrySample.hpp
  src/Domain/AxisControlCommand.hpp
  src/Domain/TelemetryConnectionStatus.hpp
  src/Domain/StandControlMode.hpp

  src/Application/Dto/PdfDocumentModel.hpp
  src/Application/Dto/PdfReportConfig.hpp
  src/Application/Dto/TelemetryConfig.hpp
  src/Application/Dto/AxisEndpointConfig.hpp

  src/Application/Ports/ILogger.hpp
  src/Application/Ports/IFunctionEngine.hpp
  src/Application/Ports/ITestExecutionScheduler.hpp
  src/Application/Ports/IPdfExporter.hpp
  src/Application/Ports/ITelemetryClient.hpp
  src/Application/Ports/IAxisProtocolCodec.hpp
  src/Application/Ports/IConfigRepository.hpp

  src/Application/Session/SessionState.hpp
  src/Application/Session/SessionState.cpp
  src/Application/Session/SessionStateData.hpp
  src/Application/Session/Subscription.hpp
  src/Application/Session/Subscription.cpp

  src/Infrastructure/Config/TomlConfigRepository.hpp
  src/Infrastructure/Config/TomlConfigRepository.cpp
  src/Infrastructure/ConfigTemplates/IAppFileLocationProvider.hpp
  src/Infrastructure/ConfigTemplates/IConfigTemplateService.hpp
  src/Infrastructure/ConfigTemplates/QtAppFileLocationProvider.hpp
  src/Infrastructure/ConfigTemplates/QtAppFileLocationProvider.cpp
  src/Infrastructure/ConfigTemplates/ConfigTemplateService.hpp
  src/Infrastructure/ConfigTemplates/ConfigTemplateService.cpp

  src/Application/ApplicationContext.hpp

  src/Application/UseCases/BuildControlPlotUseCase.hpp
  src/Application/UseCases/BuildControlPlotUseCase.cpp
  src/Application/UseCases/GenerateStairPlotUseCase.hpp
  src/Application/UseCases/GenerateStairPlotUseCase.cpp
  src/Application/UseCases/LoadPdfReportDefaultsUseCase.hpp
  src/Application/UseCases/LoadPdfReportDefaultsUseCase.cpp
  src/Application/UseCases/StartTestExecutionUseCase.hpp
  src/Application/UseCases/StartTestExecutionUseCase.cpp
  src/Application/UseCases/StopTestExecutionUseCase.hpp
  src/Application/UseCases/StopTestExecutionUseCase.cpp
  src/Application/UseCases/ExportPdfUseCase.hpp
  src/Application/UseCases/ExportPdfUseCase.cpp
  src/Application/UseCases/SetFunctionExpressionUseCase.hpp
  src/Application/UseCases/SetFunctionExpressionUseCase.cpp
  src/Application/UseCases/SetLineColorUseCase.hpp
  src/Application/UseCases/SetLineColorUseCase.cpp
  src/Application/UseCases/SetOperatorTestDurationUseCase.hpp
  src/Application/UseCases/SetOperatorTestDurationUseCase.cpp
  src/Application/UseCases/UpdateTestProtocolUseCase.hpp
  src/Application/UseCases/UpdateTestProtocolUseCase.cpp
  src/Application/UseCases/SetControlChartsTabMinutesUseCase.hpp
  src/Application/UseCases/SetControlChartsTabMinutesUseCase.cpp
  src/Application/UseCases/SetWindImpactUseCase.hpp
  src/Application/UseCases/SetWindImpactUseCase.cpp
  src/Application/UseCases/PauseTestExecutionUseCase.hpp
  src/Application/UseCases/PauseTestExecutionUseCase.cpp
  src/Application/UseCases/ResumeTestExecutionUseCase.hpp
  src/Application/UseCases/ResumeTestExecutionUseCase.cpp
  src/Application/UseCases/SetTestTimeSourceUseCase.hpp
  src/Application/UseCases/SetTestTimeSourceUseCase.cpp
  src/Application/UseCases/SetTelemetryWindowUseCase.hpp
  src/Application/UseCases/SetTelemetryWindowUseCase.cpp
  src/Application/UseCases/SetTelemetryAxisColorUseCase.hpp
  src/Application/UseCases/SetTelemetryAxisColorUseCase.cpp
  src/Application/UseCases/SetTelemetryAxisVisibleUseCase.hpp
  src/Application/UseCases/SetTelemetryAxisVisibleUseCase.cpp
  src/Application/UseCases/SetStandControlModeUseCase.hpp
  src/Application/UseCases/SetStandControlModeUseCase.cpp
  src/Application/UseCases/SetStandImpactUseCase.hpp
  src/Application/UseCases/SetStandImpactUseCase.cpp
  src/Application/UseCases/ConfigureTelemetryUseCase.hpp
  src/Application/UseCases/ConfigureTelemetryUseCase.cpp
  src/Application/UseCases/ConnectStandUseCase.hpp
  src/Application/UseCases/ConnectStandUseCase.cpp
  src/Application/UseCases/DisconnectStandUseCase.hpp
  src/Application/UseCases/DisconnectStandUseCase.cpp

  src/Infrastructure/QtTextLogger.hpp
  src/Infrastructure/QtTextLogger.cpp
  src/Infrastructure/SimpleFunctionEngine.hpp
  src/Infrastructure/QtTestExecutionScheduler.hpp
  src/Infrastructure/QtTestExecutionScheduler.cpp
  src/Infrastructure/SessionStateQtAdapter.hpp
  src/Infrastructure/SessionStateQtAdapter.cpp
  src/Infrastructure/QtPdfExporter.hpp
  src/Infrastructure/QtPdfExporter.cpp

  src/Infrastructure/AxisTcp/LegacyAxisProtocolCodec.hpp
  src/Infrastructure/AxisTcp/LegacyAxisProtocolCodec.cpp
  src/Infrastructure/AxisTcp/QtTcpTelemetryClient.hpp
  src/Infrastructure/AxisTcp/QtTcpTelemetryClient.cpp

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
  src/Presentation/ViewModels/TestTimeViewModel.hpp

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
)

target_link_libraries(develop
  PRIVATE
  Qt::Core
  Qt::Widgets
  Qt::Network
  tomlplusplus::tomlplusplus
)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CodeQuality)
enable_code_quality(develop)

if (APPLE)
  set_target_properties(develop PROPERTIES MACOSX_BUNDLE TRUE)
endif()

if (BUILD_TESTING)
  add_executable(domain_tests
    tests/Domain/AxisControlCommandTest.cpp
    tests/Domain/ControlTraceTest.cpp
    tests/Domain/ControlProfileTimingTest.cpp
    tests/Domain/ScenarioExecutionEngineTest.cpp
    tests/Domain/StandScenarioTest.cpp
    tests/Domain/StandConnectionTransitionsTest.cpp
    tests/Domain/StandImpactTransitionTest.cpp
    tests/Domain/TestExecutionPlannerTest.cpp
    tests/Domain/TestExecutionTransitionsTest.cpp
    tests/Domain/TestModeStatePolicyTest.cpp
    tests/Domain/TestProtocolTest.cpp
    tests/Domain/TimeTest.cpp
    tests/Domain/WindImpactTest.cpp
    tests/Domain/WindControlProfileImpactTest.cpp
    tests/Application/BuildControlPlotUseCaseTest.cpp
    tests/Application/ConfigureTelemetryUseCaseTest.cpp
    tests/Application/ConnectStandUseCaseTest.cpp
    tests/Application/SessionStateTest.cpp
    tests/Application/SetControlChartsTabMinutesUseCaseTest.cpp
    tests/Application/SetStandControlModeUseCaseTest.cpp
    tests/Application/SetStandImpactUseCaseTest.cpp
    tests/Application/StartTestExecutionUseCaseTest.cpp
    tests/Application/StopTestExecutionUseCaseTest.cpp
    tests/Application/UpdateTestProtocolUseCaseTest.cpp
    tests/Infrastructure/ConfigTemplateServiceTest.cpp
    tests/Infrastructure/LegacyAxisProtocolCodecTest.cpp
    tests/Infrastructure/SimpleFunctionEngineTest.cpp
    src/Application/Session/SessionState.cpp
    src/Application/Session/Subscription.cpp
    src/Application/UseCases/BuildControlPlotUseCase.cpp
    src/Application/UseCases/ConfigureTelemetryUseCase.cpp
    src/Application/UseCases/ConnectStandUseCase.cpp
    src/Application/UseCases/SetControlChartsTabMinutesUseCase.cpp
    src/Application/UseCases/SetStandControlModeUseCase.cpp
    src/Application/UseCases/SetStandImpactUseCase.cpp
    src/Application/UseCases/StartTestExecutionUseCase.cpp
    src/Application/UseCases/StopTestExecutionUseCase.cpp
    src/Application/UseCases/UpdateTestProtocolUseCase.cpp
    src/Infrastructure/ConfigTemplates/ConfigTemplateService.cpp
    src/Infrastructure/AxisTcp/LegacyAxisProtocolCodec.cpp
  )

  target_include_directories(domain_tests
    PRIVATE
      ${CMAKE_CURRENT_SOURCE_DIR}/src
  )

  target_link_libraries(domain_tests
    PRIVATE
      GTest::gtest_main
  )

  if (APPLE AND EXISTS "/opt/homebrew/opt/llvm/lib/c++")
    target_link_options(domain_tests
      PRIVATE
        -L/opt/homebrew/opt/llvm/lib/c++
        -Wl,-rpath,/opt/homebrew/opt/llvm/lib/c++
    )
  endif()

  include(GoogleTest)
  gtest_discover_tests(domain_tests)
endif()
