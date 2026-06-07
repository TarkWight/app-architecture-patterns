#ifndef TESTPROTOCOL_HPP
#define TESTPROTOCOL_HPP

#include <array>
#include <string>
#include <string_view>
#include <vector>

namespace domain {

enum class TestMode { Manual, Hybrid, Automatic };

enum class TestProgram { StabilityInIdealConditions, MaximumWindLoad, WindLoadTemporalPerspective };

constexpr TestMode testModeFromKey(std::string_view key) {
    if (key == "hybrid") {
        return TestMode::Hybrid;
    }
    if (key == "automatic") {
        return TestMode::Automatic;
    }
    return TestMode::Manual;
}

constexpr std::string_view testModeKey(TestMode mode) {
    switch (mode) {
    case TestMode::Manual:
        return "manual";
    case TestMode::Hybrid:
        return "hybrid";
    case TestMode::Automatic:
        return "automatic";
    }
    return "manual";
}

constexpr std::string_view testModeTitle(TestMode mode) {
    switch (mode) {
    case TestMode::Manual:
        return "Ручное";
    case TestMode::Hybrid:
        return "Гибридное";
    case TestMode::Automatic:
        return "Автоматическое";
    }
    return "Ручное";
}

constexpr TestProgram testProgramFromKey(std::string_view key) {
    if (key == "test2") {
        return TestProgram::MaximumWindLoad;
    }
    if (key == "test3") {
        return TestProgram::WindLoadTemporalPerspective;
    }
    return TestProgram::StabilityInIdealConditions;
}

constexpr std::string_view testProgramKey(TestProgram program) {
    switch (program) {
    case TestProgram::StabilityInIdealConditions:
        return "test1";
    case TestProgram::MaximumWindLoad:
        return "test2";
    case TestProgram::WindLoadTemporalPerspective:
        return "test3";
    }
    return "test1";
}

constexpr std::string_view testProgramTitle(TestProgram program) {
    switch (program) {
    case TestProgram::StabilityInIdealConditions:
        return "ИСПЫТАНИЕ УСТОЙЧИВОСТИ БПЛА В ИДЕАЛЬНЫХ УСЛОВИЯХ";
    case TestProgram::MaximumWindLoad:
        return "ИСПЫТАНИЕ МАКСИМАЛЬНОЙ ВЕТРОВОЙ НАГРУЗКИ БПЛА";
    case TestProgram::WindLoadTemporalPerspective:
        return "ИСПЫТАНИЕ БПЛА С УЧЕТОМ ВЕТРОВОЙ НАГРУЗКИ";
    }
    return "ИСПЫТАНИЕ УСТОЙЧИВОСТИ БПЛА В ИДЕАЛЬНЫХ УСЛОВИЯХ";
}

constexpr std::string_view testProgramShortTitle(TestProgram program) {
    switch (program) {
    case TestProgram::StabilityInIdealConditions:
        return "Полет в штиль";
    case TestProgram::MaximumWindLoad:
        return "Определение максимальных параметров";
    case TestProgram::WindLoadTemporalPerspective:
        return "Исследование временной перспективы";
    }
    return "Полет в штиль";
}

struct TestProtocolParameter {
    std::string key{};
    std::string label{};
    std::string value{};
};

struct TestProtocol {
    std::string title{};
    std::array<std::string, 8> lines{};
    TestMode testMode{TestMode::Manual};
    TestProgram testProgram{TestProgram::StabilityInIdealConditions};
    std::vector<TestProtocolParameter> droneParameters{};
};

} // namespace domain

#endif // TESTPROTOCOL_HPP
