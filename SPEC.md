# Engineering specification

## Scope and requirements

BioSecure authenticates with an ESP32-S3-connected R307, durably records attendance
offline, gives immediate non-blocking feedback, and later submits ordered events to
an HTTPS API. It supports authorized enrollment/deletion, duplicate suppression,
failed-attempt throttling and lockout, recovery, diagnostics, and a hardware-free
simulator. It never exports biometric material.

Functional requirements: verify the sensor password at boot; capture, template and
search; map sensor template IDs to application IDs; create UUID/idempotency-keyed,
monotonically sequenced events; reject same-identity events inside a configurable
window (default 45 s, boundary inclusive); append locally before success feedback;
recover complete valid records after restart; synchronize oldest-first; validate
server acknowledgements; reconnect and retry with bounded exponential backoff; mark
ACKed records; show all user states; expose metrics; require authorization for
enrollment; lock out after a configurable failure count.

Non-functional requirements: no authentication dependency on network; bounded UART
timeouts/retries, queues and storage records; deterministic degradation; no shared
mutable global state; C++17 core testable on a host; firmware watchdog-safe; secrets
provided by configuration; HTTPS certificate/hostname validation; no fabricated
performance claims.

## Hardware assumptions

ESP32-S3, 3.3 V logic; R307 UART (often 57600 8N1—confirm module); FAT-formatted
microSD over SPI; SSD1306-class I2C OLED; active-low/active-high wiring configurable;
LED and buzzer driven within board limits. R307 power may exceed a GPIO/regulator's
capacity; use a suitable supply and common ground. Pin defaults are examples only.

## Explicit state machine

States are BOOT, SELF_TEST, IDLE, CAPTURE, PROCESSING, MATCHED, DENIED, ENROLLMENT,
LOGGING, SYNCING, OFFLINE, LOCKOUT, SENSOR_ERROR, STORAGE_ERROR, NETWORK_ERROR.
The transition table in `components/core/state_machine.cpp` is authoritative and
rejects invalid transitions. Error states recover through SELF_TEST or IDLE as
defined there. Network state never blocks CAPTURE/PROCESSING.

## Protocols

R307 packets use start code `0xEF01`, 32-bit address, packet identifier, big-endian
length (payload plus two checksum bytes), payload, and checksum (identifier + length
bytes + payload). UART operations use deadlines and at most three attempts. Storage
uses newline-delimited envelopes containing canonical event JSON, CRC32, previous
SHA-256 hash and event hash. A truncated/invalid tail is quarantined on recovery.
The API is JSON over HTTPS; `Idempotency-Key` equals event UUID and an ACK is accepted
only when its UUID matches.

## Failure modes and deterministic response

Disconnected/timeout/checksum/malformed/unexpected sensor responses exhaust bounded
retries then enter SENSOR_ERROR; invalid fingers count toward throttling and LOCKOUT.
Unavailable/removed SD enters STORAGE_ERROR and may use a bounded internal emergency
queue; overflow denies recording rather than claiming success. Wi-Fi/server failures
leave durable events pending and back off. A partial final record is ignored and
quarantined. Duplicate backend submissions return the original record. Unsynchronized
time is labelled ESTIMATED if based on a prior sync plus monotonic clock, otherwise
UNKNOWN; it is never represented as accurate UTC.

## Security requirements

Follow `THREAT_MODEL.md`; validate packet sizes, checksums, TLS peers, HTTP schemas,
and record integrity; redact secrets; rate-limit attempts; audit configuration and
enrollment changes; keep credentials out of source. Hash chaining detects local
modification but does not encrypt or prevent deletion/rollback.

## Test plan and acceptance

Host tests cover packet round trips/errors/timeouts/retries; every state transition;
duplicate-window edges, UUID shape and sequencing; append/reload/corrupt/truncated
records; backoff/order/idempotency; and schema/log privacy. Backend tests cover create,
repeat and listing. CI performs host build/tests, backend checks, simulator scenarios,
format/static scans and secret-pattern checks. Acceptance additionally requires an
ESP-IDF build and the physical checklist in README; hardware-specific items remain
`PENDING HARDWARE VALIDATION` until observed.
