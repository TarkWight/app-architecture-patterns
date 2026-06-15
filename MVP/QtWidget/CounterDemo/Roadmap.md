DDD Refactoring Checklist

Goal

After protocol behavior has been fixed by tests, continue moving business logic from Application Use Cases into the Domain layer.

The goal is:

Use Case      = orchestration
Domain        = business rules
Infrastructure= TCP, files, Qt, TOML, PDF

Use Cases should coordinate actions, not perform calculations.

⸻

General Rules

Allowed in Use Cases

* Read SessionState
* Validate that operation may start
* Call Domain services/entities
* Update SessionState
* Call Ports
* Coordinate workflow

Forbidden in Use Cases

* Formula calculations
* Wind impact calculations
* Command mapping
* Beaufort conversions
* Direction conversions
* Smoothing algorithms
* Domain validation rules
* State transition rules

Those belong to Domain.

⸻

Step 1. Remove Wind Command Mapping From Use Cases — DONE: 9c10a04, 34f4b53, 1cf8b4e

Current code contains logic such as:

axis0WindCommand(...)
axis1WindCommand(...)

and direct command creation.

Move this logic into Domain.

Expected Result

Create a dedicated Domain service:

StandCommandMapper

or equivalent.

Example:

const auto commands =
    StandCommandMapper::map(windImpact);

Result:

commands.axis0
commands.axis1

Use Cases should not know how commands are calculated.

⸻

Step 2. Move Beaufort Logic Into Domain — DONE: 9c10a04, 34f4b53, 1cf8b4e

Current knowledge:

axis0 torque = beaufort * 6.5
axis1 torque = beaufort * 5.0

This is a domain rule.

Expected Result

Use Cases never perform torque calculations.

Only Domain may contain:

BeaufortForce
WindImpact
StandCommandMapper

or equivalent abstractions.

⸻

Step 3. Extract Direction Rules — DONE: 9c10a04, 34f4b53, 1cf8b4e

Current knowledge:

Direction affects axis1.position

Direction conversion rules must be owned by Domain.

Expected Result

No direct manipulation of:

command.position

inside Use Cases.

Instead:

StandCommandMapper::map(...)

returns fully configured commands.

⸻

Step 4. Extract Impact Transition Logic — DONE: existing StandImpactTransition coverage, 98312fb, 96f9d6f

Current code contains:

applyScenarioImpact(...)

and future plans:

StandImpactTransition

This belongs to Domain.

Expected Result

Use Case only performs:

transition = domainService.advance(...)

and applies result.

Use Case must not contain transition calculations.

⸻

Step 5. Remove Scenario Calculation From Use Cases — DONE: 98312fb, 96f9d6f

Current code determines:

target impact
applied impact
control trace

inside Use Cases.

Expected Result

Create Domain service:

ScenarioExecutionEngine

or equivalent.

Input:

profile
elapsedTime
currentImpact

Output:

nextImpact
traceSample

Use Case only stores result.

⸻

Step 6. Move Test Time Rules Into Domain — DONE: b8f602b, 0a9835a, a9d4860, 52e1707

Current code contains logic for:

CountUp
CountDown
remaining seconds
duration selection

inside StartTestExecutionUseCase.

Expected Result

Create Domain service:

TestExecutionPlanner

or equivalent.

Input:

protocol
time source
operator settings

Output:

duration
direction
remaining time

Use Case should not calculate those values.

⸻

Step 7. Centralize State Transition Rules — PARTIAL: bc90094, 95326b5, f321705, f0187d9, 353db98, 4899b92

Current code already contains:

canStart(...)
canStop(...)

Continue this approach.

Expected Result

All transition rules belong to Domain.

Examples:

canStart(...)
canStop(...)
canConnect(...)
canDisconnect(...)

Use Cases should only ask Domain.

⸻

Step 8. Introduce Value Objects — PARTIAL: existing WindImpact/Time value objects, 3b0ebc8, be37490, f4b7de3, 5fb7338, c0db547, ca7c2cf

Replace primitive obsession where reasonable.

Candidates:

BeaufortValue
DirectionDegrees
AngleOfAttack
WindImpact
TestDuration
ElapsedSeconds

Already completed:

* `5fb7338`
  * introduced `ControlTraceTime`;
  * removed raw trace sample time from `ControlTraceSample`.
* `c0db547`
  * introduced `WindControlSampleTime`;
  * removed independent raw `timeSeconds` / `timeMinutes` fields from `WindControlSample`.
* `ca7c2cf`
  * replaced raw `WindControlProfile::durationMinutes` with `DurationMinutes`.

Next safe value-object cleanup order:

1. `SessionState` duration setters:
   * `setEstimatedTestDurationMinutes(int)` -> `DurationMinutes`;
   * `setOperatorTestDurationMinutes(int)` -> `DurationMinutes`;
   * `setActiveTestDurationMinutes(int)` -> `DurationMinutes`.
2. `SessionState` elapsed/remaining setters:
   * `setElapsedSeconds(int)` -> `ElapsedSeconds`;
   * `setRemainingSeconds(int)` -> `RemainingSeconds`.
3. `StartTestExecutionUseCase` scheduler callback:
   * keep raw `int elapsedSeconds` at `ITestExecutionScheduler` port boundary;
   * convert to `ElapsedSeconds` immediately inside use case.
4. Telemetry value objects as separate later work:
   * `setTelemetryPollIntervalMs(int)` -> `TelemetryPollInterval`;
   * `setTelemetryWindowEndSeconds(double)` -> `TelemetryWindowEnd`.

Goal

Avoid passing raw:

double
float
int

when domain concepts already exist.

⸻

Step 9. Keep SessionState as Application State

Do NOT move SessionState into Domain.

SessionState is application state.

Domain should operate on:

entities
value objects
domain services

without knowing SessionState.

⸻

Step 10. Keep Ports in Application Layer

Do NOT move these into Domain:

ITelemetryClient
IPdfReportGenerator
IConfigTemplateService
ITestExecutionScheduler

They are application ports.

Domain must remain infrastructure-agnostic.

⸻

Step 11. Keep Qt Out Of Domain

Domain must not depend on:

QObject
QString
QTimer
QFile
QTcpSocket
QDateTime

Domain should remain pure C++.

⸻

Current Audit: Remaining Concrete Refactoring Points

This section fixes exact places found after the first DDD cleanup pass.

Already moved to Domain:

* `src/Domain/StandCommandMapper.hpp`
  * owns wind impact -> axis command mapping;
  * owns Beaufort torque coefficients;
  * owns direction -> axis1.position mapping.
* `src/Domain/TestExecutionPlanner.hpp`
  * owns start-time planning for manual/automatic/hybrid modes;
  * owns initial remaining time and completion check for running tests.
* `src/Domain/ScenarioExecutionEngine.hpp`
  * owns scenario profile -> next impact;
  * owns scenario trace sample construction.

Remaining state transition rules around mode/time-source changes:

1. `src/Application/UseCases/SetStandControlModeUseCase.cpp`
   * Current leak:
     * use case directly chooses `TestTimeSource::FreeRun` / `AutoCalculated`;
     * use case directly chooses `TestTimeDirection::CountUp` / `CountDown`.
   * Expected cleanup:
     * create/extend a Domain service, for example `TestModeStatePolicy`;
     * input: `StandControlMode`;
     * output: `TestMode`, `StandControlMode`, `TestTimeSource`, `TestTimeDirection`;
     * use case only applies returned state to `SessionState`.

2. `src/Application/UseCases/UpdateTestProtocolUseCase.cpp`
   * Current leak:
     * local helper `applyTestModeState(...)` duplicates the same mode/time-source/time-direction rules as `SetStandControlModeUseCase`.
   * Expected cleanup:
     * remove local `applyTestModeState(...)`;
     * reuse the same Domain policy as `SetStandControlModeUseCase`;
     * keep `SessionState::setTestModeState(...)` as one atomic application-state update.

3. `src/Application/UseCases/SetTestTimeSourceUseCase.cpp`
   * Current leak:
     * use case directly maps `TestTimeSource` to `TestTimeDirection`.
   * Expected cleanup:
     * move `TestTimeSource -> TestTimeDirection` rule into Domain;
     * likely place: `TestExecutionPlanner` or a smaller `TestTimePolicy`;
     * use case should only set source and derived direction returned by Domain.

4. Tests to add/update:
   * add Domain tests for the new policy:
     * manual mode -> manual stand mode, free-run, count-up;
     * hybrid mode -> hybrid stand mode, auto-calculated, count-down;
     * automatic mode -> preset scenario stand mode, auto-calculated, count-down;
     * source auto/operator -> count-down;
     * source free-run -> count-up.
   * update application tests only to verify orchestration and atomic state publication, not rule details.

Remaining StopTestExecutionUseCase time calculation:

1. `src/Application/UseCases/StopTestExecutionUseCase.cpp`
   * Current leak:
     * after stopping, use case directly checks `session.testTimeDirection`;
     * use case directly calculates `session.activeTestDuration.value() * 60`.
   * Expected cleanup:
     * move "reset time after stop" into Domain;
     * possible API:
       * `TestExecutionPlanner::resetAfterStop(activeDuration, direction)`;
       * or `TestExecutionStopPlan::from(activeDuration, direction)`;
     * output should contain:
       * `ElapsedSeconds{0}`;
       * `RemainingSeconds{duration * 60}` for countdown;
       * `RemainingSeconds{0}` for count-up.
   * Use case should only:
     * ask Domain for reset values;
     * write them to `SessionState`;
     * stop scheduler/telemetry through ports.

2. Tests to add/update:
   * Domain tests:
     * countdown stop reset restores full remaining duration;
     * count-up stop reset sets remaining to zero;
     * elapsed reset is zero in both cases.
   * Application tests:
     * keep verifying scheduler/telemetry/status orchestration;
     * avoid asserting duplicated arithmetic in the use case.

Remaining value object leaks:

1. `src/Application/Session/SessionState.hpp`
   * Current raw setters:
     * `setControlChartsTabMinutes(int minutes)`;
     * `setEstimatedTestDurationMinutes(int minutes)`;
     * `setOperatorTestDurationMinutes(int minutes)`;
     * `setActiveTestDurationMinutes(int minutes)`;
     * `setElapsedSeconds(int seconds)`;
     * `setRemainingSeconds(int seconds)`;
     * `setTelemetryWindowEndSeconds(double endSeconds)`;
     * `setTelemetryPollIntervalMs(int intervalMs)`.
   * Expected cleanup:
     * prefer overloads or replacements that accept Domain value objects:
       * `DurationMinutes`;
       * `ElapsedSeconds`;
       * `RemainingSeconds`;
       * a future `TelemetryWindowEndSeconds`;
       * a future `TelemetryPollInterval`.
     * raw `int/double` may remain at UI/Infrastructure boundaries, but should be converted before crossing into application/domain logic.

2. `src/Application/UseCases/SetControlChartsTabMinutesUseCase.cpp`
   * Current leak:
     * accepts raw `int minutes`;
     * writes same raw value to control chart duration and operator test duration.
   * Expected cleanup:
     * convert input once to `DurationMinutes`;
     * pass value object to `SessionState`.

3. `src/Application/UseCases/SetOperatorTestDurationUseCase.cpp`
   * Current leak:
     * accepts raw `int minutes`.
   * Expected cleanup:
     * convert input once to `DurationMinutes`;
     * pass value object to `SessionState`.

4. `src/Application/UseCases/StartTestExecutionUseCase.cpp`
   * Current remaining primitives:
     * `applyScenarioImpact(int elapsedSeconds)`;
     * scheduler callback receives raw `int elapsedSeconds`;
     * use case converts raw int to `ElapsedSeconds` in several places.
   * Expected cleanup:
     * keep scheduler port raw if necessary;
     * convert to `ElapsedSeconds` immediately at callback boundary;
     * pass `ElapsedSeconds` through private methods.

5. `src/Ui/MainWindow.cpp`
   * Current boundary conversion:
     * `applyStandInputs()` calls `domain::makeWindImpact(double, double, double)` from widget values.
   * This is acceptable as UI boundary code for MVP.
   * Future cleanup:
     * consider replacing `makeWindImpact(double, double, double)` call sites with explicit `Beaufort::from`, `WindDirection::from`, `AngleOfAttack::from` if construction rules become more complex.

6. `src/Domain/WindImpact.hpp`
   * Current helper:
     * `makeWindImpact(double beaufort, double direction, double angleOfAttack)`.
   * This is convenient, but still allows primitive construction.
   * Future cleanup:
     * either keep it as a boundary factory;
     * or introduce an explicit `WindImpact::from(Beaufort, WindDirection, AngleOfAttack)` and limit raw factory usage to UI/tests.

7. `src/Application/Ports/ITestExecutionScheduler.hpp`
   * Current API:
     * `start(int initialElapsedSeconds, TickCallback onTick)`;
     * `TickCallback` emits raw `int`.
   * This is an application port boundary.
   * Future cleanup:
     * consider using `ElapsedSeconds` in the application port if Qt adapter remains simple;
     * otherwise convert raw values immediately in use cases.

8. `src/Application/Ports/ITelemetryClient.hpp`
   * Current API:
     * `startPolling(int intervalMs)`.
   * This is an infrastructure-facing port boundary.
   * Future cleanup:
     * introduce `TelemetryPollInterval` value object if interval validation grows beyond current clamp in `SessionState`.

⸻

Final Target

Desired shape:

UI
 ↓
Use Case
 ↓
Domain Service
 ↓
Domain Objects
Use Case
 ↓
Port
 ↓
Infrastructure

Use Cases become short.

Most business behavior lives in Domain.

Infrastructure contains only technical details.
