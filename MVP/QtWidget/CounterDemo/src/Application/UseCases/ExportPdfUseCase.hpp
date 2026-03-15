#ifndef EXPORTPDFUSECASE_HPP
#define EXPORTPDFUSECASE_HPP

#include <string>

#include "../Dto/PdfDocumentModel.hpp"
#include "../../Application/Ports/IPdfExporter.hpp"
#include "../Session/SessionState.hpp"

namespace application::useCases {

class ExportPdfUseCase final {
  public:
    ExportPdfUseCase(application::session::SessionState &state, application::ports::IPdfExporter &pdfExporter);

    application::dto::PdfDocumentModel buildDocument() const;
    void execute(const std::string &filePath) const;

  private:
    application::session::SessionState &state;
    application::ports::IPdfExporter &pdfExporter;
};

} // namespace application::useCases

#endif // EXPORTPDFUSECASE_HPP
