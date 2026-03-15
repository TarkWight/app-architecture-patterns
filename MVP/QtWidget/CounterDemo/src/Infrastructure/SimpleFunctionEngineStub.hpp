#ifndef SIMPLEFUNCTIONENGINESTUB_HPP
#define SIMPLEFUNCTIONENGINESTUB_HPP

#include "../Application/Ports/IFunctionEngine.hpp"
#include <cmath>
#include <string>

namespace infrastructure {

class SimpleFunctionEngineStub final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string &expr, double x) const override {
        if (expr == "sin" || expr == "sin(x)") {
            return std::sin(x);
        }
        if (expr == "cos" || expr == "cos(x)") {
            return std::cos(x);
        }
        if (expr == "x" || expr == "line") {
            return x;
        }
        if (expr == "x^2" || expr == "x*x" || expr == "square") {
            return x * x;
        }
        if (expr == "step") {
            const int value = static_cast<int>(x) % 8;
            return static_cast<double>(value);
        }

        return std::sin(x);
    }
};

} // namespace infrastructure

#endif // SIMPLEFUNCTIONENGINESTUB_HPP
