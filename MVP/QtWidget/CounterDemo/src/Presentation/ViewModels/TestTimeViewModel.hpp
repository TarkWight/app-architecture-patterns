#ifndef TESTTIMEVIEWMODEL_HPP
#define TESTTIMEVIEWMODEL_HPP

#include "../../Domain/TestExecutionStatus.hpp"
#include "../../Domain/TestTimeDirection.hpp"
#include "../../Domain/TestTimeSource.hpp"

namespace presentation::viewModels {

struct TestTimeViewModel {
    domain::TestExecutionStatus executionStatus{domain::TestExecutionStatus::Idle};

    domain::TestTimeSource timeSource{domain::TestTimeSource::FreeRun};
    domain::TestTimeDirection timeDirection{domain::TestTimeDirection::CountUp};

    int estimatedDurationMinutes{20};
    int operatorDurationMinutes{20};
    int activeDurationMinutes{20};

    int elapsedSeconds{0};
    int remainingSeconds{0};

    int displayedSeconds{0};
};

} // namespace presentation::viewModels

#endif // TESTTIMEVIEWMODEL_HPP