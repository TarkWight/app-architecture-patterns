#ifndef PDFDOCUMENTMODEL_HPP
#define PDFDOCUMENTMODEL_HPP

#include <array>
#include <string>
#include <vector>

#include "../../Domain/Plot.hpp"
#include "../../Domain/TestProtocol.hpp"

namespace application::dto {

struct PdfReportTableRow final {
    std::string testType{};
    std::string windFormula{};
    std::string estimatedDurationMinutes{};
    std::string testDurationMinutes{};
    std::string result{};
};

struct PdfDocumentModel final {
    std::string title{};

    std::string organization{};
    std::string licenseNumber{};
    std::string address{};
    std::string testMode{};
    std::string testProgramTitle{};
    std::string testProgramShortTitle{};
    std::string reportDate{};
    std::string operatorName{};
    std::string comment{};
    std::vector<domain::TestProtocolParameter> droneParameters{};
    std::string conclusion{};

    std::vector<PdfReportTableRow> tableRows{};

    std::array<std::string, 4> firstStanza{};
    domain::PlotModel telemetryPlot{};
    std::string telemetryPlotCaption{"Рисунок 2 - Изменение положения стенда во время теста."};

    domain::PlotModel controlPlot{};
    std::string controlPlotCaption{"Рисунок 1 - Изменение силы ветра во время теста."};
    std::array<std::string, 4> secondStanza{};
};

} // namespace application::dto

#endif // PDFDOCUMENTMODEL_HPP
