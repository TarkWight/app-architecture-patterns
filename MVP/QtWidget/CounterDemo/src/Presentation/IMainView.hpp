#ifndef IMAINVIEW_HPP
#define IMAINVIEW_HPP

#include <string>
#include "../Domain/CounterId.hpp"

namespace presentation {

class IMainView {
  public:
    virtual ~IMainView() = default;

    virtual void setCounterValue(domain::CounterId id, int value) = 0;
    virtual void appendCommandLog(const std::string &text) = 0;
};

} // namespace presentation

#endif // IMAINVIEW_HPP
