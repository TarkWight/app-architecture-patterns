#ifndef SESSIONSTATE_HPP
#define SESSIONSTATE_HPP

#include "../../Domain/Plot.hpp"

#include "SessionStateData.hpp"
#include "Subscription.hpp"

#include <functional>
#include <mutex>
#include <string>
#include <unordered_map>

namespace application::session {

class SessionState {
  public:
    using Listener = std::function<void(const SessionStateData &)>;

    const SessionStateData &get() const;

    Subscription subscribe(Listener listener);

    void setFunctionExpression(std::string expr);

    void setLineColor(domain::RgbColor color);

    void setControlChartsTabMinutes(int minutes);

    void setTimerDurationMinutes(int minutes);

    void setElapsedSeconds(int seconds);

    void setTimerRunning(bool running);

    void setPlot1(domain::PlotModel plot);

    void setPlot2(domain::PlotModel plot);

    void setPoemTitle(std::string title);

    void setPoemLine(int idx, std::string line);

  private:
    SessionStateData data{};

    mutable std::mutex mu{};
    std::unordered_map<long long, Listener> listeners{};
    long long lastId{0};

    void notify();
};

} // namespace application::session

#endif // SESSIONSTATE_HPP
