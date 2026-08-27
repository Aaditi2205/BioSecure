# Biometric privacy boundary

Fingerprint images, raw samples, extracted features and R307 templates remain inside
the R307 sensor whenever its supported capture/match/enrollment commands are used.
They are forbidden from ESP storage, microSD, event records, APIs, backend databases,
logs, metrics, tests and fixtures. The application handles only a sensor template ID,
an optional application user ID, outcome, time metadata and device/event identifiers.

The packet driver intentionally exposes commands that ask the sensor to convert and
match internally; it exposes no upload/download-template API. Debug logging records
command names and result codes, never packet payloads. Deletion removes the sensor
slot and separately audits its numeric ID. Identity mappings should be minimized,
access-controlled and retained only as policy requires. This architecture is informed
by biometric information-protection principles and is not a compliance certification.
