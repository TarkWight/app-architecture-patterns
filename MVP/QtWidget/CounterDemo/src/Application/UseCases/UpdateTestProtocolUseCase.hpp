#ifndef UPDATEPOEMUSECASE_HPP
#define UPDATEPOEMUSECASE_HPP

#include "../Session/SessionState.hpp"

#include <string>

namespace application::useCases {

class UpdateTestProtocolUseCase final {
  public:
    explicit UpdateTestProtocolUseCase(application::session::SessionState &state);

    void updateTitle(std::string title);
    void updateLine(int index, std::string line);
    void updateMode(std::string mode);
    void updateProgram(std::string program);
    void updateDroneParameterValue(int index, std::string value);

  private:
    application::session::SessionState &state;
};

} // namespace application::useCases

#endif // UPDATEPOEMUSECASE_HPP
