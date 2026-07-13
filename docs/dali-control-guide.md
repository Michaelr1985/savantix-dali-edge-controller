# DALI control guide

This guide documents the controls found in `dali_sweep_results.csv` and the firmware API added to expose them.

## What changed

The firmware now has a `dali::control::DaliControl` service in
`components/dali_control`. It converts short-address, group, broadcast, arc-level,
and special commands into the existing queued DALI transaction pipeline. Commands
are retried and transmitted through whichever PHY is configured by the application.

The command catalogue is in
`components/dali_protocol/include/dali/protocol/dali_commands.h` and frame encoding
is in `dali_frame.h`.

## Basic usage

```cpp
#include "dali/control/dali_control.h"

auto address = dali::ShortAddress::fromRaw(3U);
auto group = dali::GroupAddress::fromRaw(2U);
dali::control::DaliControl control{pipeline};

control.sendShort(*address, dali::protocol::DaliCommand::GoToScene7);
control.setShortLevel(*address, 170U);       // arc level 0..254
control.sendGroup(*group, dali::protocol::DaliCommand::AddToGroup2);
control.setGroupLevel(*group, 120U);
control.sendBroadcast(dali::protocol::DaliCommand::Off);
```

`setShortLevel` and `setGroupLevel` reject 255 because 255 is not a valid direct arc
level. Check the returned `ControlResult` (`Ok`, `QueueFull`, `TransactionFailed`,
or `InvalidArgument`) and surface failures to the supervisory layer.

## Configuration example

DALI configuration commands use DTR0 as their parameter. Set DTR0 with the special
command, then send the configuration command to the target gear:

```cpp
control.setDtr0(0x2AU);
control.sendShort(*address, dali::protocol::DaliCommand::StoreDtrAsMaxLevel);
control.sendShort(*address, dali::protocol::DaliCommand::SavePersistentVariables);
```

The same pattern applies to minimum level, power-on level, system-failure level,
fade time/rate, extended fade time, scene levels, group membership, short address,
operating mode, and memory-bank reset. Use high priority for commissioning traffic.

## Controls discovered by the sweep

| Range | Firmware names | Purpose |
| --- | --- | --- |
| `0x00..0x0A` | `Off`, `Up`, `Down`, step/recall commands | Immediate lighting control |
| `0x10..0x1F` | `GoToScene0..15` | Recall a scene |
| `0x20..0x2E` | reset, DTR/configuration commands | Configure persistent gear values |
| `0x40..0x7F` | store/remove scenes and groups | Commission scenes and membership |
| `0x80..0x81` | store short address, enable memory write | Addressing and memory access |
| `0x90..0xC5` | query, scene/group, random-address, memory commands | Read status and device data |

All standard opcodes observed in the CSV are represented by typed enum values. DALI
special commands (initialise, randomise, search-address, program/verify short address,
and device-type selection) are represented by `DaliSpecialCommand` and sent with:

```cpp
control.sendSpecial(dali::protocol::DaliSpecialCommand::Initialise, 0x00U);
```

## Sweep interpretation and limitations

The sweep found replies for 42 commands, five memory banks marked present, and many
no-reply results. A no-reply result means that this particular device/test sequence
did not return a response; it is not proof that the command is unsupported. Repeat
queries on the installed gear with the correct device type, timing, and response
window before treating them as unavailable.

Observed memory banks:

- Bank 0: present, last location `0x1A`.
- Bank 1: present, last location `0x77` (luminaire identification extension).
- Bank 202: present, last location `0x0F` (mandatory active energy/power data).
- Banks 203 and 204: absent in this sweep (optional energy/power data).
- Banks 205 and 206: present, last locations `0x1C` and `0x20` (control-gear and
  light-source diagnostics).

`ReadMemoryLocation` and the memory enable/reset commands are available as raw
protocol controls, but the application does not yet decode banks 202/205/206 into
`LightRealtimeData`. The next integration step is a scheduled memory reader with
bank-specific decoders and validation against the target gear.

The sweep also reported no replies for several failure queries, manufacturer-specific
mode, and memory reads. Keep those probes behind diagnostics/commissioning controls
until they are validated on hardware.

## Integration notes

`DaliControl` uses the existing `DaliProtocolPipeline`, command queue, transaction
timeouts, retry policy, and PHY abstraction. The current application still boots with
the simulator PHY; replace it with the production DALI PHY in `main/app_main.cpp` when
the UART/timer transport is connected. Keep one owner for DALI bus transactions and
call the control service from that owner rather than writing directly to the PHY.

## Verification

The host suite covers frame encoding and control-service behavior. Run:

```sh
cmake -S test/host -B build-host -DCMAKE_BUILD_TYPE=Debug
cmake --build build-host --parallel
ctest --test-dir build-host --output-on-failure
```

The ESP-IDF application also builds with the new component enabled.
