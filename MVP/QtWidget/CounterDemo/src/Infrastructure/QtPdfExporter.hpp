#ifndef QTPDFEXPORTER_HPP
#define QTPDFEXPORTER_HPP

#include "../Application/Ports/IPdfExporter.hpp"

namespace infrastructure {

class QtPdfExporter final : public application::ports::IPdfExporter {
  public:
    QtPdfExporter() = default;

    void exportDocument(const application::dto::PdfDocumentModel &document, const std::string &filePath) override;
};

} // namespace infrastructure

#endif // QTPDFEXPORTER_HPP
