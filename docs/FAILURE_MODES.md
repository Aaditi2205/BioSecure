# Failure modes

| Fault | Detection | Response | Recovery |
|---|---|---|---|
| Sensor absent/UART timeout | deadline | bounded retry, SENSOR_ERROR | periodic self-test |
| Bad checksum/frame | codec | discard, bounded retry | next valid frame/self-test |
| Unknown finger | ACK code | deny, count attempt | retry delay/lockout expiry |
| SD absent | mount/append error | bounded emergency queue, warning | remount then drain |
| Partial/corrupt log | CRC/parser/chain | stop at last valid record, quarantine tail | operator inspect |
| Wi-Fi/server down | event/timeout | remain offline, exponential backoff | reconnect, ordered drain |
| Bad/mismatched ACK | schema/UUID | retain pending | retry |
| Never-synced clock | time service | UNKNOWN timestamp quality | SNTP sync |
| Queue full | nonblocking send | metric + safe denial/defer by queue policy | consumer drain |
| Reboot mid-write | missing newline/CRC | ignore incomplete tail | replay valid pending records |
