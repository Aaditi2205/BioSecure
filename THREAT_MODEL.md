# Threat model

Assets: identity mapping, attendance history, device/API credentials, configuration,
availability, and biometric templates held by the R307. Trust boundaries exist at
the sensor UART, removable SD card, provisioning interface, Wi-Fi/TLS network, API,
and physical enclosure.

Threats include UART injection/corruption, spoof fingers, repeated guessing, stolen
SD modification, record deletion/rollback, replay/duplicate HTTP requests, malicious
responses, credential disclosure, unauthorized enrollment, clock manipulation,
resource exhaustion, and device tampering. Controls include strict framing and bounded
lengths, lockout/throttling, authorized enrollment, append CRC plus optional SHA-256
chain, idempotency, TLS verification, ACK validation, bounded queues/retries, explicit
time quality and audit events.

Residual risks: R307 authenticity and presentation attacks, physical extraction,
UART substitution, SD deletion/rollback (hash chaining cannot prevent either), denial
of service, compromised firmware/backend, and privacy risk from identifiers. Secure
boot, flash encryption, signed OTA, a secure element, tamper switches, server-side
chain anchoring and a certified PAD-capable sensor are commercial hardening work.
This design is informed by biometric information-protection principles; it claims no
ISO certification.
