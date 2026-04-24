#include "ExportPdfUseCase.hpp"

namespace application::useCases {

ExportPdfUseCase::ExportPdfUseCase(application::session::SessionState &state,
                                   application::ports::IPdfExporter &pdfExporter)
    : state(state), pdfExporter(pdfExporter) {
}

application::dto::PdfDocumentModel ExportPdfUseCase::buildDocument() const {
    const auto &session = state.get();

    application::dto::PdfDocumentModel document{};
    document.title = session.testProtocol.title;

    for (int i = 0; i < 4; ++i) {
        document.firstStanza[static_cast<std::size_t>(i)] = session.testProtocol.lines[static_cast<std::size_t>(i)];
    }

    document.telemetryPlot = session.telemetryPlot;
    document.telemetryPlotCaption = "Рисунок 1";

    document.controlPlot = session.controlPlot;
    document.controlPlotCaption = "Рисунок 2";

    for (std::size_t i = 0; i < 4; ++i) {
        document.secondStanza[i] = session.testProtocol.lines[i + 4];
    }

    return document;
}

void ExportPdfUseCase::execute(const std::string &filePath) const {
    const auto document = buildDocument();
    pdfExporter.exportDocument(document, filePath);
}

} // namespace application::useCases
