#pragma once
#include "r307_packet.hpp"
#include <chrono>
#include <cstdint>
#include <vector>

namespace biosecure::r307 {
enum class Command : std::uint8_t { CaptureImage=0x01, ImageToTemplate=0x02, Search=0x04, CreateModel=0x05, StoreModel=0x06, DeleteModel=0x0C, TemplateCount=0x1D, VerifyPassword=0x13 };
enum class Confirmation : std::uint8_t { Ok=0x00, PacketError=0x01, NoFinger=0x02, ImageFail=0x03, NoMatch=0x09, NotFound=0x09, BadLocation=0x0B, FlashError=0x18, Unknown=0xFF };
enum class DriverError { None, Timeout, Transport, Malformed, UnexpectedPacket, AddressMismatch, EmptyAck, RetryLimit };
struct OperationResult { Confirmation confirmation{Confirmation::Unknown}; DriverError error{DriverError::None}; std::vector<std::uint8_t> data; std::uint8_t attempts{0}; bool ok() const { return error == DriverError::None && confirmation == Confirmation::Ok; } };

class IByteTransport {
public:
    virtual ~IByteTransport() = default;
    virtual bool write(const std::uint8_t* data, std::size_t size) = 0;
    virtual bool readPacket(std::vector<std::uint8_t>& data, std::chrono::milliseconds timeout) = 0;
};

class Driver {
public:
    explicit Driver(IByteTransport& transport, std::uint32_t address=kDefaultAddress, std::uint8_t retries=2, std::chrono::milliseconds timeout=std::chrono::milliseconds{500});
    OperationResult verifyPassword(std::uint32_t password);
    OperationResult captureImage();
    OperationResult imageToTemplate(std::uint8_t buffer_id);
    OperationResult searchTemplate(std::uint8_t buffer_id, std::uint16_t start, std::uint16_t count);
    OperationResult createModel();
    OperationResult storeModel(std::uint8_t buffer_id, std::uint16_t location);
    OperationResult deleteModel(std::uint16_t location, std::uint16_t count=1);
    OperationResult getTemplateCount();
private:
    OperationResult execute(Command command, const std::vector<std::uint8_t>& args);
    IByteTransport& transport_; std::uint32_t address_; std::uint8_t retries_; std::chrono::milliseconds timeout_;
};
}
