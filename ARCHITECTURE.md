# Architecture

The dependency rule is ports and adapters. Pure C++ domain services own policy;
ESP-IDF adapters own UART, SPI/FATFS, Wi-Fi/HTTPS, clocks, GPIO and FreeRTOS. The R307
packet codec is pure C++; its driver consumes an injected bounded byte transport.

`FingerprintTask` exclusively owns the sensor. `AttendanceTask` owns suppression,
sequence allocation and event creation. `StorageTask` exclusively owns the log.
`NetworkTask` owns Wi-Fi/SNTP/HTTPS and drains durable records. `UserInterfaceTask`
owns OLED/LED/buzzer. Bounded queues transfer immutable value messages; an event group
publishes WIFI_UP, TIME_SYNCED, STORAGE_READY and SHUTDOWN; direct notifications only
wake an owning task. Overflow policies are documented beside queue definitions.

Persistence happens before success UI. ACKs flow Network → Storage; sensor bytes can
never reach attendance/storage/network messages. A mutex is limited to metrics
snapshots. Shutdown and watchdog polling are bounded. Interfaces make all policies
host-testable with fake transports, clocks, logs and clients.
