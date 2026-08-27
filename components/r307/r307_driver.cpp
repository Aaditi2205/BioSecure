#include "r307_driver.hpp"

namespace biosecure::r307 {
namespace { void add16(std::vector<std::uint8_t>& v, std::uint16_t n) { v.push_back(static_cast<std::uint8_t>(n >> 8)); v.push_back(static_cast<std::uint8_t>(n)); } void add32(std::vector<std::uint8_t>& v, std::uint32_t n) { v.push_back(static_cast<std::uint8_t>(n >> 24)); v.push_back(static_cast<std::uint8_t>(n >> 16)); v.push_back(static_cast<std::uint8_t>(n >> 8)); v.push_back(static_cast<std::uint8_t>(n)); } }
Driver::Driver(IByteTransport& t, std::uint32_t a, std::uint8_t r, std::chrono::milliseconds timeout) : transport_(t), address_(a), retries_(r), timeout_(timeout) {}
OperationResult Driver::execute(Command command, const std::vector<std::uint8_t>& args) {
    Packet command_packet{address_, PacketId::Command, {static_cast<std::uint8_t>(command)}};
    command_packet.payload.insert(command_packet.payload.end(), args.begin(), args.end());
    const auto wire = serialize(command_packet); DriverError last = DriverError::RetryLimit;
    for (std::uint8_t attempt = 1; attempt <= static_cast<std::uint8_t>(retries_ + 1); ++attempt) {
        if (!transport_.write(wire.data(), wire.size())) { last = DriverError::Transport; continue; }
        std::vector<std::uint8_t> response;
        if (!transport_.readPacket(response, timeout_)) { last = DriverError::Timeout; continue; }
        auto parsed = deserialize(response);
        if (parsed.error != ParseError::None) { last = DriverError::Malformed; continue; }
        if (parsed.packet.id != PacketId::Ack) { last = DriverError::UnexpectedPacket; continue; }
        if (parsed.packet.address != address_) { last = DriverError::AddressMismatch; continue; }
        if (parsed.packet.payload.empty()) { last = DriverError::EmptyAck; continue; }
        OperationResult result; result.confirmation = static_cast<Confirmation>(parsed.packet.payload.front()); result.attempts = attempt;
        result.data.assign(parsed.packet.payload.begin() + 1, parsed.packet.payload.end()); return result;
    }
    OperationResult failed; failed.error = last; failed.attempts = retries_ + 1; return failed;
}
OperationResult Driver::verifyPassword(std::uint32_t p) { std::vector<std::uint8_t> a; add32(a,p); return execute(Command::VerifyPassword,a); }
OperationResult Driver::captureImage() { return execute(Command::CaptureImage,{}); }
OperationResult Driver::imageToTemplate(std::uint8_t b) { return execute(Command::ImageToTemplate,{b}); }
OperationResult Driver::searchTemplate(std::uint8_t b,std::uint16_t s,std::uint16_t c) { std::vector<std::uint8_t> a{b}; add16(a,s); add16(a,c); return execute(Command::Search,a); }
OperationResult Driver::createModel() { return execute(Command::CreateModel,{}); }
OperationResult Driver::storeModel(std::uint8_t b,std::uint16_t l) { std::vector<std::uint8_t> a{b}; add16(a,l); return execute(Command::StoreModel,a); }
OperationResult Driver::deleteModel(std::uint16_t l,std::uint16_t c) { std::vector<std::uint8_t> a; add16(a,l); add16(a,c); return execute(Command::DeleteModel,a); }
OperationResult Driver::getTemplateCount() { return execute(Command::TemplateCount,{}); }
}
