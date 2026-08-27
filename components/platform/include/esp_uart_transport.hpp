#pragma once
#include "r307_driver.hpp"
#include "driver/uart.h"
#include <cstdint>

namespace biosecure::platform {
struct UartConfig { uart_port_t port{UART_NUM_1}; int tx_pin{17}; int rx_pin{18}; int baud{57600}; std::size_t rx_buffer{1024}; };
class EspUartTransport final : public r307::IByteTransport {
public:
    explicit EspUartTransport(UartConfig config):config_(config){}
    ~EspUartTransport() override;
    bool initialize();
    bool write(const std::uint8_t* data,std::size_t size) override;
    bool readPacket(std::vector<std::uint8_t>& data,std::chrono::milliseconds timeout) override;
private:
    bool readExact(std::uint8_t* out,std::size_t size,std::int64_t deadline_us);
    UartConfig config_; bool initialized_{false};
};
}
