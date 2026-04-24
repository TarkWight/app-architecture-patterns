#ifndef PDFDOCUMENTMODEL_HPP
#define PDFDOCUMENTMODEL_HPP

#include <array>
#include <string>

#include "../../Domain/Plot.hpp"

namespace application::dto {

struct PdfDocumentModel final {
    std::string title{};

    std::array<std::string, 4> firstStanza{};
    domain::PlotModel telemetryPlot{};
    std::string telemetryPlotCaption{"Рисунок 1"};

    domain::PlotModel controlPlot{};
    std::string controlPlotCaption{"Рисунок 2"};
    std::array<std::string, 4> secondStanza{};
};

} // namespace application::dto

#endif // PDFDOCUMENTMODEL_HPP
