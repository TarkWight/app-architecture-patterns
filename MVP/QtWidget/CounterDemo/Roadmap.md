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

Step 7. Centralize State Transition Rules — DONE: bc90094, 95326b5, f321705, f0187d9, 353db98, 4899b92

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

Step 8. Introduce Value Objects — PARTIAL: existing WindImpact/Time value objects, 3b0ebc8, be37490, f4b7de3, 5fb7338, c0db547, ca7c2cf, 4a69f17, d5bba07, 3b878fa, c460e1b

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
* `4a69f17`
  * replaced `SessionState` duration setters with `DurationMinutes`.
* `d5bba07`
  * replaced `SessionState` elapsed/remaining setters with `ElapsedSeconds` / `RemainingSeconds`.
* `3b878fa`
  * replaced telemetry poll interval setter with `TelemetryPollInterval`.
* `c460e1b`
  * replaced telemetry window end setter with `TelemetryWindowEnd`.
* `7d58dfd`
  * replaced `SetOperatorTestDurationUseCase::execute(int)` with `execute(DurationMinutes)`;
  * moved raw UI `int` conversion to presentation boundary.
* current cleanup
  * removed `SessionState::setControlChartsTabMinutes(int)` compatibility overload.

Next safe value-object cleanup order:

No immediate value-object cleanup remains inside the application/domain flow.

Explicitly deferred:

* `ITestExecutionScheduler`
  * keeps raw `int elapsedSeconds` at the infrastructure-facing port boundary;
  * `StartTestExecutionUseCase` converts it to `ElapsedSeconds` immediately.
* `ITelemetryClient::startPolling`
  * keeps raw `int intervalMs` at the infrastructure-facing port boundary.

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
  * owns effective wind direction -> axis1.position mapping.
* `src/Domain/EffectiveWindDirection.hpp`
  * owns `normalize(windDirection + angleOfAttack + yawOscillationOffset)`;
  * keeps angle of attack as a relative direction offset, not a sector width.
* `src/Domain/TestExecutionPlanner.hpp`
  * owns start-time planning for manual/automatic/hybrid modes;
  * owns initial remaining time and completion check for running tests;
  * owns stop-time reset through `resetAfterStop(...)`.
* `src/Domain/ScenarioExecutionEngine.hpp`
  * owns scenario profile -> next impact;
  * owns scenario trace sample construction.
* `src/Domain/TestModeStatePolicy.hpp`
  * owns test mode -> stand mode/time source/time direction mapping;
  * owns operator duration availability and derived time direction rules.

Remaining value object leaks:

1. `src/Application/Session/SessionState.hpp`
   * `setControlChartsTabMinutes(int minutes)` compatibility overload removed.
   * Current status:
     * stores `DurationMinutes`;
     * accepts `DurationMinutes` from application use cases.

2. `src/Application/UseCases/SetControlChartsTabMinutesUseCase.cpp`
   * Already accepts `DurationMinutes`.
   * Remaining check:
     * keep this use case as orchestration only; do not move UI raw values into it.

3. `src/Application/UseCases/SetOperatorTestDurationUseCase.cpp`
   * Done in `7d58dfd`.
   * Current status:
     * accepts `DurationMinutes`;
     * presentation boundary converts widget `int` to `DurationMinutes`.

4. `src/Application/UseCases/StartTestExecutionUseCase.cpp`
   * Scheduler callback still receives raw `int elapsedSeconds` from `ITestExecutionScheduler`.
   * Current status:
     * raw port value is converted to `ElapsedSeconds` immediately inside the callback;
     * private scenario logic already receives `ElapsedSeconds`.
   * Future cleanup:
     * consider changing the port itself to emit `ElapsedSeconds` if Qt adapter remains simple.

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
   * This is an infrastructure-facing port boundary and remains raw by design for MVP.
   * Rule:
     * convert to `ElapsedSeconds` immediately after the callback enters application logic.

8. `src/Application/Ports/ITelemetryClient.hpp`
   * Current API:
     * `startPolling(int intervalMs)`.
   * This is an infrastructure-facing port boundary and remains raw by design for MVP.

Remaining MVP behavior gaps:

1. Drone passport data for future calculations
   * Current status:
     * `pdf_report.drone_parameters` are treated as source/passport data for the BAS/UAV under test, not as PDF-only fields;
     * application layer maps protocol drone parameters into domain `UavSpecification`;
     * domain does not know TOML, `pdf_report` section names or raw string values.
   * Expected cleanup:
     * future scenario duration and disturbance calculators must use `UavSpecification`;
     * do not calculate from raw TOML/PDF fields directly;
     * add confirmed formulas for mass, battery/time, DR and payload effects when the model is known.

2. Dynamic yaw oscillation
   * Current status:
     * `YawOscillationOffset` exists as a domain value object;
     * `EffectiveWindDirection` and `StandCommandMapper` support it.
     * `YawOscillationPolicy` produces deterministic sinusoidal offsets over time;
     * MVP model is a deterministic Beaufort-dependent sinusoid:
       `yawOscillationOffset = amplitude(beaufort) * sin(2π * elapsed / 12s)`;
     * MVP amplitude is `beaufort * 1.5°`;
     * this is MVP calibration, not a validated physical model;
     * `UavSpecification` is present in `StandImpactCalculationContext`, but does not affect current yaw calculation.
   * Legacy note:
     * recovered legacy `SetRouteSpin` behavior is a protocol observation, not domain truth;
     * legacy UI actions overwrite the same transport field, so angle/yaw behavior must not be copied as a domain model.
   * Expected cleanup:
     * post-MVP amplitude/frequency may depend on `UavSpecification` fields after a physical or empirical model is defined:
       mass, DR, payload, motor and battery parameters.

3. Confirm legacy operational limits
   * Current TODOs:
     * signed angle of attack `-360..360`.
   * Confirmed:
     * Beaufort `0..7` is an explicit operational limit; drones are not tested above 7.
   * Expected cleanup:
     * verify real stand constraints for signed angle of attack and replace MVP guard if needed.

4. Additional impact smoothing
   * Current status:
     * real stand/business flow already prevents unsafe hard jumps during tests.
   * Decision:
     * do not add another smoothing layer in MVP.

5. Hybrid manual override return
   * Current status:
     * verified during stand testing: the system returns to the correct state without an additional custom return policy.
   * Decision:
     * do not add a separate hybrid-return mechanism in MVP.

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
