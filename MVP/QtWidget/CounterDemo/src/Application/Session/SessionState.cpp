#include "SessionState.hpp"

#include "../../Domain/Plot.hpp"
#include "../../Domain/Poem.hpp"
#include "../../Domain/Time.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <array>
#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>
#include <utility>

namespace application::session {

const application::session::SessionStateData &application::session::SessionState::get() const {
    return data;
}

Subscription application::session::SessionState::subscribe(Listener listener) {
    std::lock_guard lock(mu);
    const auto id = ++lastId;
    listeners.emplace(id, std::move(listener));

    listeners.at(id)(data);

    return Subscription([this, id] {
        std::lock_guard lock(mu);
        listeners.erase(id);
    });
}

void application::session::SessionState::setFunctionExpression(std::string expr) {
    data.functionExpression.value = std::move(expr);
    notify();
}

void application::session::SessionState::setWindProfile(domain::WindProfile profile) {
    data.windProfile = std::move(profile);
    notify();
}

void application::session::SessionState::setLineColor(domain::RgbColor color) {
    data.lineColor = color;
    notify();
}

void application::session::SessionState::setControlChartsTabMinutes(int minutes) {
    data.controlChartsTabMinutes.value = minutes;
    notify();
}

void application::session::SessionState::setTimerDurationMinutes(int minutes) {
    data.timerDuration.value = minutes;
    notify();
}

void application::session::SessionState::setElapsedSeconds(int seconds) {
    data.elapsed.value = seconds;
    notify();
}

void application::session::SessionState::setTimerRunning(bool running) {
    data.timerRunning = running;
    notify();
}

void application::session::SessionState::setPlot1(domain::PlotModel plot) {
    data.plot1 = std::move(plot);
    notify();
}

void application::session::SessionState::setPlot2(domain::PlotModel plot) {
    data.plot2 = std::move(plot);
    notify();
}

void application::session::SessionState::setPoemTitle(std::string title) {
    data.poem.title = std::move(title);
    notify();
}

void application::session::SessionState::setPoemLine(int idx, std::string line) {
    if (idx < 0 || idx >= 8) {
        return;
    }
    data.poem.lines[static_cast<std::size_t>(idx)] = std::move(line);
    notify();
}

void application::session::SessionState::notify() {
    std::unordered_map<long long, Listener> copy;
    {
        std::lock_guard lock(mu);
        copy = listeners;
    }
    for (auto &[id, listener] : copy) {
        listener(data);
    }
}

} // namespace application::session
