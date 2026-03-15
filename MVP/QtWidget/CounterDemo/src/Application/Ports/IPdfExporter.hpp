#ifndef IPDFEXPORTER_HPP
#define IPDFEXPORTER_HPP

#include <string>

#include "../Dto/PdfDocumentModel.hpp"

namespace application::ports {

class IPdfExporter {
  public:
    virtual ~IPdfExporter() = default;

    virtual void exportDocument(const application::dto::PdfDocumentModel &document, const std::string &filePath) = 0;
};

} // namespace application::ports

#endif // IPDFEXPORTER_HPP
