#ifndef IFUNCTIONENGINE_HPP
#define IFUNCTIONENGINE_HPP

#include <string>

namespace application::ports {

class IFunctionEngine {
  public:
    virtual ~IFunctionEngine() = default;

    virtual double eval(const std::string &expr, double x) const = 0;
};

} // namespace application::ports

#endif // IFUNCTIONENGINE_HPP
