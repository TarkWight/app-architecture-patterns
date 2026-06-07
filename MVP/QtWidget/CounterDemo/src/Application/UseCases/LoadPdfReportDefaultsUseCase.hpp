#ifndef LOADPDFREPORTDEFAULTSUSECASE_HPP
#define LOADPDFREPORTDEFAULTSUSECASE_HPP

#include "../Ports/IConfigRepository.hpp"
#include "../Session/SessionState.hpp"

#include <string>

namespace application::useCases {

class LoadPdfReportDefaultsUseCase final {
  public:
    LoadPdfReportDefaultsUseCase(application::session::SessionState &state,
                                 application::ports::IConfigRepository &configRepository);

    void execute(const std::string &configPath);
    void applyEmptyDefaults();
    void saveEmptyTemplate(const std::string &configPath);

  private:
    application::session::SessionState &state;
    application::ports::IConfigRepository &configRepository;
};

} // namespace application::useCases

#endif // LOADPDFREPORTDEFAULTSUSECASE_HPP
