# Savantix DALI Phase 2 Protocol Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add standard DALI forward-frame construction, backward-response decoding, a fixed-capacity priority command queue, bounded retries with jittered backoff, and communication-quality tracking on top of the verified PHY.

**Architecture:** `dali_protocol` is portable C++17. A serial `DaliTransactionService` is the only protocol object that invokes `IDaliPhy`; delay and jitter are injected so host tests never sleep. Commands enter a fixed 64-entry stable priority queue. Typed results preserve timeout, collision, bus, and transceiver failures rather than collapsing them into missing-device state.

**Tech Stack:** C++17, CMake/CTest, ESP-IDF v6.0.2, existing `IDaliPhy` and simulator

---

## File map

| File | Responsibility |
| --- | --- |
| `components/dali_protocol/include/dali/protocol/dali_commands.h` | Standard command byte constants |
| `components/dali_protocol/include/dali/protocol/dali_frame.h` | Broadcast, group, and short forward frames |
| `components/dali_protocol/include/dali/protocol/dali_response_parser.h` | Basic status decoding |
| `components/dali_protocol/include/dali/protocol/dali_command_queue.h` | Stable fixed-capacity priority queue |
| `components/dali_protocol/include/dali/protocol/dali_transaction.h` | Request/result, retry, delay, jitter, quality, service APIs |
| `components/dali_protocol/dali_response_parser.cpp` | Status parser |
| `components/dali_protocol/dali_transaction.cpp` | Transaction/retry implementation |
| `test/host/test_dali_frame.cpp` | Address and frame encoding |
| `test/host/test_dali_response_parser.cpp` | Response and status flags |
| `test/host/test_dali_command_queue.cpp` | Priority, FIFO stability, capacity |
| `test/host/test_dali_transaction.cpp` | Success, timeout, retries, backoff, collision and bus failure |
| `test/host/test_communication_quality.cpp` | Counters and success-rate calculation |

### Task 1: Construct standards-based forward frames

**Files:**
- Create: `components/dali_protocol/CMakeLists.txt`
- Create: `components/dali_protocol/include/dali/protocol/dali_commands.h`
- Create: `components/dali_protocol/include/dali/protocol/dali_frame.h`
- Create: `test/host/test_dali_frame.cpp`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write failing frame tests**

Test these exact encodings:

```cpp
CHECK_EQ(makeShortCommand(*ShortAddress::fromRaw(3), DaliCommand::QueryStatus).raw,
         0x0790U);
CHECK_EQ(makeShortArc(*ShortAddress::fromRaw(3),
                      *ArcLevel::fromRaw(127U)).raw, 0x067FU);
CHECK_EQ(makeGroupCommand(*GroupAddress::fromRaw(2), DaliCommand::Off).raw,
         0x8500U);
CHECK_EQ(makeGroupArc(*GroupAddress::fromRaw(2),
                      *ArcLevel::fromRaw(254U)).raw, 0x84FEU);
CHECK_EQ(makeBroadcastCommand(DaliCommand::QueryStatus).raw, 0xFF90U);
CHECK_EQ(makeBroadcastArc(*ArcLevel::fromRaw(42U)).raw, 0xFE2AU);
```

Also assert that direct-arc level 255 is rejected because it is the mask value, and that a raw frame round-trips its address/data bytes.

- [ ] **Step 2: Verify missing-header failure**

```bash
cmake -S test/host -B build/host
cmake --build build/host --parallel
```

Expected: failure naming `dali/protocol/dali_frame.h`.

- [ ] **Step 3: Implement commands and frame helpers**

Define `DaliCommand : uint8_t` with `Off=0x00`, `QueryStatus=0x90`, `QueryControlGearPresent=0x91`, `QueryLampFailure=0x92`, `QueryLampPowerOn=0x93`, `QueryLimitError=0x94`, `QueryResetState=0x95`, `QueryMissingShortAddress=0x96`, `QueryVersionNumber=0x97`, `QueryDeviceType=0x99`, `QueryPhysicalMinimum=0x9A`, `QueryActualLevel=0xA0`, `QueryMaxLevel=0xA1`, `QueryMinLevel=0xA2`, `QueryPowerOnLevel=0xA3`, `QuerySystemFailureLevel=0xA4`, `QueryFadeTimeRate=0xA5`, `QueryGroups0To7=0xC0`, `QueryGroups8To15=0xC1`, and `QueryRandomAddressH/M/L=0xC2/0xC3/0xC4`.

Define `ForwardFrame { uint16_t raw; addressByte(); dataByte(); }`, `ArcLevel::fromRaw(0..254)`, and constexpr helpers using the standard address bytes: short command `(address<<1)|1`, short arc `address<<1`, group command `0x81|(group<<1)`, group arc `0x80|(group<<1)`, broadcast command `0xFF`, and broadcast arc `0xFE`.

- [ ] **Step 4: Verify and commit**

```bash
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
git add components/dali_protocol test/host
git commit -m "feat: construct DALI forward frames"
```

### Task 2: Decode backward responses and basic status

**Files:**
- Create: `components/dali_protocol/include/dali/protocol/dali_response_parser.h`
- Create: `components/dali_protocol/dali_response_parser.cpp`
- Create: `test/host/test_dali_response_parser.cpp`
- Modify: `components/dali_protocol/CMakeLists.txt`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write failing parser tests**

Verify that raw `0xFF` produces a valid yes response, raw `0x00` produces a valid no response, and status byte `0b11111111` maps bits 0-7 to control-gear failure, lamp failure, lamp power on, limit error, fade running, reset state, missing short address, and power failure. Verify a response marked `PhyResult::Collision`, `Timeout`, or `TransceiverFault` is not parsed as a valid value.

- [ ] **Step 2: Observe missing parser failure**

Run the specific new target and expect a missing-header failure.

- [ ] **Step 3: Implement typed parsing**

Define `BackwardResponse { uint8_t value; ReadingQuality quality; }`, `BasicStatus` with the eight named booleans, `parseYesNo`, and `parseBasicStatus`. The API accepts only a successful PHY result as valid and otherwise returns `ReadingQuality::TemporarilyUnavailable` for timeout/busy/collision, or `Invalid` for hardware/internal errors.

- [ ] **Step 4: Verify and commit**

Run all host tests, then commit `feat: parse DALI backward responses`.

### Task 3: Add the bounded stable priority queue

**Files:**
- Create: `components/dali_protocol/include/dali/protocol/dali_command_queue.h`
- Create: `test/host/test_dali_command_queue.cpp`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write failing queue tests**

Use priorities `Critical`, `High`, `Normal`, and `Low`. Enqueue two normal requests, one low, and one critical; assert critical dequeues first, equal-priority requests remain FIFO, and low dequeues last. Fill all 64 entries and assert enqueue 65 returns `QueueResult::Full`. Assert pop on empty returns no value.

- [ ] **Step 2: Verify missing queue failure**

Run the new target and observe the missing header.

- [ ] **Step 3: Implement without heap allocation**

Define `DaliRequest` with frame, expects-response flag, timeout, maximum retries, priority, device key, and request sequence. Implement `DaliCommandQueue` using `std::array<DaliRequest,64>` plus count. Insert after all existing items of equal or higher priority so equal priority remains FIFO. Reject when full; pop shifts remaining bounded entries.

- [ ] **Step 4: Verify and commit**

Run all host tests and the ESP-IDF build, then commit `feat: add bounded DALI command queue`.

### Task 4: Execute transactions with bounded retry and jitter

**Files:**
- Create: `components/dali_protocol/include/dali/protocol/dali_transaction.h`
- Create: `components/dali_protocol/dali_transaction.cpp`
- Create: `test/host/test_dali_transaction.cpp`
- Modify: `components/dali_protocol/CMakeLists.txt`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write failing transaction tests**

Use real `SimulatedDaliPhy`, `RecordingDelay`, and `FixedJitter`. Cover:

- successful query in one attempt with returned byte;
- no-response command that transmits once and does not call receive;
- two timeouts followed by success, producing three attempts and delays `100+jitter`, `500+jitter`;
- all retries exhausted after four total attempts when `maxRetries=3`;
- collision and busy are retryable;
- bus-power loss and transceiver fault stop immediately;
- a request timeout of zero is rejected without touching the PHY.

- [ ] **Step 2: Verify missing transaction failure**

Run the target and observe the missing API.

- [ ] **Step 3: Implement injected timing and jitter**

Define `IDelayProvider::delayMs`, `IJitterSource::next(maxInclusive)`, `TransactionStatus`, `DaliTransactionResult` with optional response, attempts, last PHY result, and retry delay total. `DaliTransactionService::execute` transmits then receives when requested. Retryable results are timeout, busy, and collision. Base delays are 100, 500, and 2000 ms; retries beyond index 2 use 2000 ms. Add jitter in 0-25 ms. The service must never retry bus power loss, transceiver fault, invalid argument, not initialised, or internal error.

- [ ] **Step 4: Verify and commit**

Run all host tests and commit `feat: execute bounded DALI transactions`.

### Task 5: Track communication quality and errors

**Files:**
- Modify: `components/dali_protocol/include/dali/protocol/dali_transaction.h`
- Modify: `components/dali_protocol/dali_transaction.cpp`
- Create: `test/host/test_communication_quality.cpp`
- Modify: `test/host/CMakeLists.txt`

- [ ] **Step 1: Write failing quality tests**

Record two successes, one timeout, one collision, one busy result, and one transceiver failure. Expect total 6, success 2, each named counter 1, and success rate `2/6`. A new tracker reports 1.0 success rate only when explicitly queried with no transactions as `noData=true`; it must not imply healthy communications.

- [ ] **Step 2: Observe missing tracker behavior**

Run the target and expect compilation failure for `CommunicationQualityTracker`.

- [ ] **Step 3: Implement saturating counters**

Add `CommunicationQualityCounters` and `CommunicationQualitySnapshot`. Increment 32-bit counters with saturation at `UINT32_MAX`. Derive failure count and floating success rate without division by zero. `record` accepts the final transaction result and classifies its last PHY outcome.

- [ ] **Step 4: Verify and commit**

Run all host tests and commit `feat: track DALI communication quality`.

### Task 6: Integrate and document Phase 2

**Files:**
- Modify: `main/CMakeLists.txt`
- Modify: `main/app_main.cpp`
- Modify: `README.md`
- Create: `docs/verification/phase-2.md`

- [ ] **Step 1: Write a failing protocol integration test**

Compose queue, simulator, transaction service, and parser: enqueue a `QueryStatus` request for short address 3, script response `0x04`, execute it, and assert status parsing reports lamp power on with no failures. Assert the quality snapshot contains one success.

- [ ] **Step 2: Observe failure before integration helper exists**

The test includes `dali/protocol/dali_protocol_pipeline.h`; verify that missing helper causes the red state.

- [ ] **Step 3: Implement the minimal pipeline**

Add a pipeline owning the queue, transaction service reference, and quality tracker. `processNext()` returns empty when the queue is empty and otherwise executes exactly one request, records quality, and returns the request/result pair. It does not schedule or continuously poll.

- [ ] **Step 4: Wire the boot self-check**

In demo mode only, enqueue one simulated `QueryStatus`, script `0x04`, process once, and log success at info level. Do not claim discovery or a real light. Add `dali_protocol` to main dependencies.

- [ ] **Step 5: Run fresh verification**

```bash
cmake --fresh -S test/host -B build/host -DCMAKE_BUILD_TYPE=Debug
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
. "$HOME/esp/esp-idf-v6.0.2/export.sh"
idf.py -B build/idf build
git diff --check
```

Expected: all Phase 1 and Phase 2 tests pass, firmware builds for ESP32-S3, and no diff errors exist.

- [ ] **Step 6: Record evidence and commit**

Document exact test inventory, image size, failure semantics, retry timings, and remaining hardware timing limitations. Commit as `feat: complete DALI protocol phase`.

## Phase 2 completion gate

- [ ] Frame encodings match standard short, group, and broadcast address-byte rules.
- [ ] Mask value 255 cannot be sent as a direct arc level.
- [ ] PHY failures remain distinguishable from valid zero/false backward responses.
- [ ] Priority queue capacity is fixed and FIFO-stable within priority.
- [ ] Retries are bounded, jittered, and never applied to non-retryable hardware failures.
- [ ] Communication quality has explicit no-data state and saturating counters.
- [ ] Host tests and ESP-IDF build pass from fresh build directories.
- [ ] Verification evidence is committed.
