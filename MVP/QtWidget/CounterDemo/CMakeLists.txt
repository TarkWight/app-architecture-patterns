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
  src/Domain/Poem.hpp
  src/Domain/Time.hpp



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



  src/Application/UseCases/BuildFormulaPlotUseCase.hpp
  src/Application/UseCases/BuildFormulaPlotUseCase.cpp


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
  src/Application/UseCases/UpdatePoemUseCase.hpp
  src/Application/UseCases/UpdatePoemUseCase.cpp
  src/Application/UseCases/SetTab2MinutesUseCase.hpp
  src/Application/UseCases/SetTab2MinutesUseCase.cpp


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
  src/Presentation/Tab1/ITelemetryChartsTabView.hpp
  src/Presentation/Tab2/ITab2View.hpp
  src/Presentation/Tab3/ITab3View.hpp
  src/Presentation/ShellPresenter.hpp
  src/Presentation/ShellPresenter.cpp
  src/Presentation/Tab1/Tab1Presenter.hpp
  src/Presentation/Tab1/Tab1Presenter.cpp
  src/Presentation/Tab2/Tab2Presenter.hpp
  src/Presentation/Tab2/Tab2Presenter.cpp
  src/Presentation/Tab3/Tab3Presenter.hpp
  src/Presentation/Tab3/Tab3Presenter.cpp

  src/Ui/MainWindowUiAdapter.hpp
  src/Ui/MainWindowUiAdapter.cpp
  src/Ui/Tab1Widget.hpp
  src/Ui/Tab1Widget.cpp
  src/Ui/Tab2Widget.hpp
  src/Ui/Tab2Widget.cpp
  src/Ui/Tab3Widget.hpp
  src/Ui/Tab3Widget.cpp
  src/Ui/MainWindow.hpp
  src/Ui/MainWindow.cpp

  src/Ui/MainWindow.ui
  src/Ui/Tab1Widget.ui
  src/Ui/Tab2Widget.ui
  src/Ui/Tab3Widget.ui
  src/Application/ApplicationContext.hpp
  src/Ui/PlotWidget.hpp src/Ui/PlotWidget.cpp
  src/Ui/Render/PlotRenderer.hpp src/Ui/Render/PlotRenderer.cpp
)
target_link_libraries(develop PRIVATE Qt::Core Qt::Widgets)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(CodeQuality)
enable_code_quality(develop)

if (APPLE)
  set_target_properties(develop PROPERTIES MACOSX_BUNDLE TRUE)
endif()
