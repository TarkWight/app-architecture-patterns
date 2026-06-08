#ifndef SIMPLEFUNCTIONENGINE_HPP
#define SIMPLEFUNCTIONENGINE_HPP

#include "../Application/Ports/IFunctionEngine.hpp"

#include <cmath>
#include <cstddef>
#include <string>
#include <string_view>

namespace infrastructure {

class SimpleFunctionEngine final : public application::ports::IFunctionEngine {
  public:
    double eval(const std::string &expr, double x) const override {
        Parser parser{expr, x};
        return parser.eval();
    }

  private:
    class Parser final {
      public:
        Parser(std::string_view expression, double variableValue)
            : expression(expression), variableValue(variableValue) {
        }

        double eval() {
            const double value = parseExpression();
            skipSpaces();
            return hasError || position != expression.size() ? 0.0 : value;
        }

      private:
        std::string_view expression;
        double variableValue{0.0};
        std::size_t position{0};
        bool hasError{false};

        double parseExpression() {
            double value = parseTerm();
            while (!hasError) {
                if (consume('+')) {
                    value += parseTerm();
                } else if (consume('-')) {
                    value -= parseTerm();
                } else {
                    break;
                }
            }
            return value;
        }

        double parseTerm() {
            double value = parsePower();
            while (!hasError) {
                if (consume('*')) {
                    value *= parsePower();
                } else if (consume('/')) {
                    const double divisor = parsePower();
                    value = divisor == 0.0 ? fail() : value / divisor;
                } else {
                    break;
                }
            }
            return value;
        }

        double parsePower() {
            double value = parseUnary();
            if (consumeText("**") || consume('^')) {
                value = std::pow(value, parsePower());
            }
            return value;
        }

        double parseUnary() {
            if (consume('+')) {
                return parseUnary();
            }
            if (consume('-')) {
                return -parseUnary();
            }
            return parsePrimary();
        }

        double parsePrimary() {
            if (consume('(')) {
                const double value = parseExpression();
                return consume(')') ? value : fail();
            }
            consumeText("std::");
            if (consumeName("sin")) {
                return std::sin(parseFunctionArgument());
            }
            if (consumeName("cos")) {
                return std::cos(parseFunctionArgument());
            }
            if (consumeName("abs")) {
                return std::abs(parseFunctionArgument());
            }
            if (consumeName("x") || consumeName("t")) {
                return variableValue;
            }
            return parseNumber();
        }

        double parseFunctionArgument() {
            if (!consume('(')) {
                return fail();
            }
            const double value = parseExpression();
            return consume(')') ? value : fail();
        }

        double parseNumber() {
            skipSpaces();
            const std::size_t start = position;
            bool hasDigit = false;
            bool hasDot = false;
            while (position < expression.size() && (isDigit(expression[position]) || expression[position] == '.')) {
                hasDigit = hasDigit || isDigit(expression[position]);
                if (expression[position] == '.') {
                    if (hasDot) {
                        return fail();
                    }
                    hasDot = true;
                }
                ++position;
            }
            if (start == position || !hasDigit) {
                return fail();
            }

            return std::stod(std::string{expression.substr(start, position - start)});
        }

        bool consume(char character) {
            skipSpaces();
            if (position >= expression.size() || expression[position] != character) {
                return false;
            }
            ++position;
            return true;
        }

        bool consumeText(std::string_view text) {
            skipSpaces();
            if (expression.substr(position, text.size()) != text) {
                return false;
            }
            position += text.size();
            return true;
        }

        bool consumeName(std::string_view name) {
            skipSpaces();
            if (expression.substr(position, name.size()) != name) {
                return false;
            }
            position += name.size();
            return true;
        }

        void skipSpaces() {
            while (position < expression.size() && expression[position] == ' ') {
                ++position;
            }
        }

        double fail() {
            hasError = true;
            return 0.0;
        }

        static bool isDigit(char character) {
            return character >= '0' && character <= '9';
        }
    };
};

} // namespace infrastructure

#endif // SIMPLEFUNCTIONENGINE_HPP
