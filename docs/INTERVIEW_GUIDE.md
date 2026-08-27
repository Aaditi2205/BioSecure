# Interview guide

1. **Why UART for R307?** The module exposes a framed asynchronous serial command
   interface; `components/r307` implements that contract directly and isolates it
   behind `IByteTransport`.
2. **Asynchronous serial?** Sender and receiver share no clock line. Both preconfigure
   rate/format and recover byte timing from start bits; typical setup is 57600 8N1.
3. **Baud rate?** Symbols per second. With ordinary UART one symbol carries one bit,
   so 57600 baud is 57600 bit/s, with framing overhead per byte.
4. **Framing?** `EF01`, address, identifier, payload-plus-checksum length, payload and
   checksum. `r307_packet.cpp` uses big-endian helpers and an input bound.
5. **Checksums?** UART supplies byte transport, not whole-message integrity. The
   checksum detects corruption before an ACK or match result is trusted; it is not
   cryptographic authentication.
6. **UART vs SPI vs I2C?** UART is point-to-point asynchronous and simple. SPI is
   synchronous, full-duplex and fast with per-device chip selects. I2C is addressed,
   two-wire and convenient for low-bandwidth peripherals.
7. **Why SPI for SD?** It provides practical block-storage bandwidth and FATFS support
   with deterministic ownership, while I2C remains available for the OLED.
8. **Sensor disconnect?** The fingerprint task hits a deadline, retries a fixed count,
   enters SENSOR_ERROR and periodically self-tests. Other tasks remain scheduled.
9. **Wi-Fi failure?** Events are already persisted. Network task moves offline and
   backs off; authentication and UI continue, then oldest-first draining resumes.
10. **Why no network dependency?** Availability and user latency must not inherit WAN
    behavior. Local authentication and append establish the immediate result.
11. **Why FreeRTOS?** It separates timing/failure domains and peripheral ownership;
    UART stalls do not serialize UI, storage or reconnection work.
12. **Tasks/queues/synchronization?** Tasks are scheduled execution contexts; bounded
    queues transfer messages; event groups publish coarse readiness; notifications
    wake a specific owner. See `ARCHITECTURE.md`.
13. **Race avoidance?** Each peripheral and mutable policy has one task owner. Other
    tasks exchange immutable value messages. Only metric snapshots need a short lock.
14. **Duplicate attendance?** `AttendanceService` stores last accepted monotonic time
    per template ID and suppresses elapsed times `<=` the configured window; tests
    exercise at and one millisecond past the boundary.
15. **Why templates stay in sensor?** It minimizes exposure and keeps application,
    SD, API and logs outside the biometric-material trust boundary. The driver does
    not expose template transfer operations.
16. **Offline sync?** Storage appends first. Network reads pending records in sequence,
    submits with the UUID idempotency key, validates the matching ACK and marks it.
17. **Idempotency?** A retry with the same key returns the original database record;
    unique `(device_id, sequence)` also detects divergent replay.
18. **Security limits?** Physical/UART substitution, sensor spoofing, SD deletion or
    rollback, DoS and compromised endpoints remain. The threat model lists hardening.
19. **Why no liveness claim?** No evidence establishes robust PAD for this module.
    Rate controls limit abuse but cannot distinguish a live finger from an artifact.
20. **Commercial changes?** Evaluated PAD plus second factor, secure boot/encrypted
    storage/signed OTA, secure element, tamper detection, provisioned device identity,
    server-anchored audit chain, privacy lifecycle controls, fleet observability and
    environmental/security validation.

Strong discussion ties choices to failure containment: direct packet parsing makes
corruption testable; interfaces make time/transport/storage deterministic on a host;
bounded queues and retry caps protect memory and scheduling; sequence plus UUID solves
ordering and at-least-once delivery without exporting biometric material.
