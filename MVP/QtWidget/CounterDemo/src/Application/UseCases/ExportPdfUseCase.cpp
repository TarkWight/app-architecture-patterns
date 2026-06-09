#include "ExportPdfUseCase.hpp"

#include <array>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>

namespace application::useCases {

namespace {

std::string valueOrDefault(const std::string &value, const std::string &defaultValue) {
    return value.empty() ? defaultValue : value;
}

std::string todayDate() {
    const std::time_t currentTime = std::time(nullptr);
    std::tm localTime{};
#if defined(_WIN32)
    localtime_s(&localTime, &currentTime);
#else
    localtime_r(&currentTime, &localTime);
#endif

    std::ostringstream out;
    out << std::put_time(&localTime, "%d-%m-%Y");
    return out.str();
}

} // namespace

ExportPdfUseCase::ExportPdfUseCase(application::session::SessionState &state,
                                   application::ports::IPdfExporter &pdfExporter)
    : state(state), pdfExporter(pdfExporter) {
}

application::dto::PdfDocumentModel ExportPdfUseCase::buildDocument() const {
    const auto &session = state.get();
    const auto &lines = session.testProtocol.lines;

    application::dto::PdfDocumentModel document{};
    document.title = valueOrDefault(session.testProtocol.title, "Протокол испытаний беспилотной авиационной системы");
    document.organization = valueOrDefault(lines[0], "Организация не указана");
    document.licenseNumber = valueOrDefault(lines[1], "не указан");
    document.address = valueOrDefault(lines[2], "Адрес не указан");
    document.testMode = domain::testModeTitle(session.testProtocol.testMode);
    document.testProgramTitle = domain::testProgramTitle(session.testProtocol.testProgram);
    document.testProgramShortTitle = domain::testProgramShortTitle(session.testProtocol.testProgram);
    document.operatorName = valueOrDefault(lines[3], "не указан");
    document.comment = lines[4];
    document.conclusion = valueOrDefault(lines[5], "Заключение не указано");
    document.droneParameters = session.testProtocol.droneParameters;
    document.reportDate = todayDate();

    for (int i = 0; i < 4; ++i) {
        document.firstStanza[static_cast<std::size_t>(i)] = lines[static_cast<std::size_t>(i)];
    }

    application::dto::PdfReportTableRow row{};
    row.testType = document.testProgramShortTitle;
    row.windFormula = valueOrDefault(session.functionExpression.value, "0");
    row.estimatedDurationMinutes = std::to_string(session.estimatedTestDuration.value());
    row.testDurationMinutes = std::to_string(session.activeTestDuration.value());
    row.result = valueOrDefault(lines[6], "_________");
    document.tableRows.push_back(std::move(row));

    document.telemetryPlot = session.telemetryPlot;
    document.telemetryPlotCaption = "Рисунок 2 - Изменение положения стенда во время теста.";

    document.controlPlot = session.controlPlot;
    document.controlPlotCaption = "Рисунок 1 - Изменение силы ветра во время теста.";

    for (std::size_t i = 0; i < 4; ++i) {
        document.secondStanza[i] = lines[i + 4];
    }

    return document;
}

void ExportPdfUseCase::execute(const std::string &filePath) const {
    const auto document = buildDocument();
    pdfExporter.exportDocument(document, filePath);
}

} // namespace application::useCases
