# Savantix DALI Visual Simulator Site Design

**Date:** 2026-07-10  
**Status:** Approved direction; implementation pending written-spec review  
**Location:** `simulator/`  
**Purpose:** Browser-based visual testing surface for the ESP32-S3 DALI edge-controller model

## 1. Goal

Create a standalone local web site that makes the DALI simulator observable and testable without an ESP32 board. The site presents four simulated lights, their health and bus state, active events, and a replayable timeline. It must let a user inject realistic faults, watch persistence and recovery behavior, pause and accelerate time, and inspect the same kinds of event snapshots emitted by the firmware model.

This is a test and demonstration tool, not a replacement for the ESP-IDF firmware and not a production lighting-control UI.

## 2. Approved product shape

The site uses the approved hybrid structure:

- **Plant overview home:** four light nodes, bus load, overall health, active event count, simulation clock, and scenario controls.
- **Diagnostic console:** selecting a light opens its detailed health, command level, current/power/temperature trend, communication quality, persistence state, and event history.
- **Timeline command center:** a replay view correlates polling, measurements, fault activation, escalation, clearing, and recovery over time.

The home view is the default; the diagnostic and timeline views are tabs or mode switches within the same app shell so the user does not lose simulation state while navigating.

## 3. Visual direction

Use a dark industrial-control visual language derived from the approved companion direction:

- near-black blue background;
- deep slate panels and thin blue-gray dividers;
- mint/teal for healthy or connected state;
- amber for warning/thermal drift;
- red for confirmed fault or missing state;
- monospace for telemetry, addresses, timestamps, and values;
- clean sans-serif for headings and explanatory text;
- restrained 12–18 px radii, compact controls, and open grid spacing;
- no decorative gradients, marketing hero, or unrelated imagery.

The visual hierarchy prioritizes the simulation clock and overall state, then four lights, then event stream and controls. Color is paired with text and iconography so state is not color-only.

## 4. App shell and components

```text
SimulatorApp
├── TopBar
│   ├── product title
│   ├── connection/demo status
│   ├── simulation clock and playback controls
│   └── scenario selector
├── OverviewView
│   ├── SummaryStrip
│   ├── PlantMap
│   │   └── LightNode x4
│   ├── EventRail
│   └── FaultControls
├── DiagnosticView
│   ├── DeviceHeader
│   ├── HealthScorePanel
│   ├── TelemetryChart
│   ├── CapabilityPanel
│   └── DeviceEventHistory
└── TimelineView
    ├── TimeAxis
    ├── PollTrack
    ├── MeasurementTrack
    ├── FaultTrack
    └── EventInspector
```

React components remain focused. The simulation engine, scenario definitions, event reducer, and chart data transformations live outside the view components. `App` composes views; it does not own a monolithic event loop.

## 5. Simulation model

The initial scenario contains exactly four lights matching the firmware demo:

| Address | Seed state | Behavior |
| --- | --- | --- |
| 01 | Healthy | stable level/current/temperature and healthy communication |
| 02 | Lamp failure | commanded on, direct lamp-failure flag, score falls to failed |
| 03 | Thermal drift | temperature rises through warning and critical thresholds, then recovers when cleared |
| 04 | Intermittent communication | response failures alternate, communication quality and event persistence change |

Simulation time is virtual and advances only while playing. Speed choices are 0.5×, 1×, 4×, and 20×. A deterministic tick reducer produces readings, scheduler polls, baseline updates, diagnostics, health evaluations, events, and C6 messages. The browser model mirrors firmware semantics but is intentionally a pure TypeScript test double; it does not call ESP-IDF C++ directly.

Each simulated reading carries timestamp, value, validity, and quality. Unsupported values remain unavailable. Event deduplication uses address plus event family. The user can pause, resume, reset, select a scenario, inject a fault, clear a fault, and jump to the next event.

## 6. Interaction requirements

- Clicking a light node opens its diagnostic view.
- “Inject fault” offers lamp failure, gear failure, thermal warning, thermal critical, current drift, power drift, missing device, and communication degradation.
- “Clear fault” starts the same healthy persistence/recovery window used by the simulator model; it does not instantly restore health to 100.
- Pause freezes virtual time and all derived state.
- Speed changes affect virtual time only, not event ordering or deterministic values.
- Reset returns to the selected scenario’s seed state and clears event history.
- Selecting a timeline event opens the corresponding device and highlights the event snapshot.
- A “send to C6” indicator increments only for event/state-change/summary/request messages; normal repeated readings remain local.
- Controls have keyboard focus, visible selected/focus states, and text labels.

## 7. Data contracts

The TypeScript model mirrors these firmware concepts:

- `LightState`, `EventSeverity`, `LightEventType`;
- `LightRealtimeData` with validity-bearing readings;
- `HealthEvaluation` with score, state, and active fault flags;
- `SimulationEvent` with timestamp, address, type, severity, snapshot, and description;
- `C6Message` with type, sequence, payload summary, and send status;
- `SimulationState` with clock, playback, speed, selected address, lights, events, timeline, and counters.

Scenario and reducer logic must be deterministic under a supplied seed or fixed initial state so tests can assert exact event sequences.

## 8. Responsive behavior

- Desktop: three-column overview rhythm with plant map, event rail, and controls.
- Tablet: plant map remains two columns; event rail moves below controls.
- Mobile: one-column light list, sticky playback strip, diagnostic panels stack, and timeline scrolls horizontally without page overflow.

No critical value, event severity, or control may be hidden only on mobile.

## 9. Testing and verification

Unit tests cover reducer ticks, scenario reset, fault injection/clear persistence, health recovery, event deduplication/escalation, C6 forwarding policy, virtual-time speed, and timeline ordering. Browser tests cover selecting a light, injecting a lamp fault, pausing, clearing it, observing score recovery, opening the timeline event, and resetting the scenario.

Visual QA checks the accepted hybrid layout at desktop and mobile sizes, with no clipped telemetry, unreadable controls, or accidental horizontal overflow. The site runs from the simulator folder with documented install, development, test, and production-build commands.

## 10. Non-goals

- No live serial/UART connection in the first site release.
- No Thread, cloud, authentication, or server backend.
- No direct control of real DALI lights.
- No invented firmware behavior beyond the documented simulator contract.
- No raster artwork is required; the visual language is code-native telemetry UI.
