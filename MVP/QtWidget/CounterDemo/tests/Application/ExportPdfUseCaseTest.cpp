#include "../../src/Application/UseCases/ExportPdfUseCase.hpp"

#include "../../src/Application/Ports/IPdfExporter.hpp"
#include "../../src/Application/Session/SessionState.hpp"

#include <gtest/gtest.h>

namespace {

class PdfExporterSpy final : public application::ports::IPdfExporter {
  public:
    void exportDocument(const application::dto::PdfDocumentModel &document, const std::string &filePath) override {
        lastDocument = document;
        lastFilePath = filePath;
    }

    application::dto::PdfDocumentModel lastDocument{};
    std::string lastFilePath{};
};

TEST(ExportPdfUseCaseTest, UsesProtocolStateModeAndProgramForPdfDocument) {
    application::session::SessionState state{};
    state.setTestProtocolMode(domain::TestMode::Automatic);
    state.setTestProtocolProgram(domain::TestProgram::MaximumWindLoad);
    PdfExporterSpy pdfExporter{};
    application::useCases::ExportPdfUseCase useCase{state, pdfExporter};

    const auto document = useCase.buildDocument();

    EXPECT_EQ(document.testMode, "Автоматическое");
    EXPECT_EQ(document.testProgramTitle, "ИСПЫТАНИЕ МАКСИМАЛЬНОЙ ВЕТРОВОЙ НАГРУЗКИ БПЛА");
    EXPECT_EQ(document.testProgramShortTitle, "Определение максимальных параметров");
}

} // namespace
