#include "r307_packet.hpp"

namespace biosecure::r307 {
namespace {
void put16(std::vector<std::uint8_t>& out, std::uint16_t v) { out.push_back(static_cast<std::uint8_t>(v >> 8)); out.push_back(static_cast<std::uint8_t>(v & 0xff)); }
void put32(std::vector<std::uint8_t>& out, std::uint32_t v) { out.push_back(static_cast<std::uint8_t>(v >> 24)); out.push_back(static_cast<std::uint8_t>(v >> 16)); out.push_back(static_cast<std::uint8_t>(v >> 8)); out.push_back(static_cast<std::uint8_t>(v)); }
std::uint16_t get16(const std::vector<std::uint8_t>& b, std::size_t i) { return (std::uint16_t(b[i]) << 8) | b[i + 1]; }
std::uint32_t get32(const std::vector<std::uint8_t>& b, std::size_t i) { return (std::uint32_t(b[i]) << 24) | (std::uint32_t(b[i+1]) << 16) | (std::uint32_t(b[i+2]) << 8) | b[i+3]; }
bool valid_id(std::uint8_t id) { return id == 1 || id == 2 || id == 7 || id == 8; }
}
std::uint16_t checksum(PacketId id, std::uint16_t len, const std::vector<std::uint8_t>& payload) {
    std::uint32_t sum = static_cast<std::uint8_t>(id) + (len >> 8) + (len & 0xff);
    for (auto byte : payload) sum += byte;
    return static_cast<std::uint16_t>(sum);
}
std::vector<std::uint8_t> serialize(const Packet& packet) {
    if (packet.payload.size() > kMaxPayloadSize) return {};
    const auto len = static_cast<std::uint16_t>(packet.payload.size() + 2);
    std::vector<std::uint8_t> out; out.reserve(kFixedPrefixSize + len);
    put16(out, kStartCode); put32(out, packet.address); out.push_back(static_cast<std::uint8_t>(packet.id)); put16(out, len);
    out.insert(out.end(), packet.payload.begin(), packet.payload.end()); put16(out, checksum(packet.id, len, packet.payload));
    return out;
}
ParseResult deserialize(const std::vector<std::uint8_t>& b) {
    ParseResult r;
    if (b.size() < 11) { r.error = ParseError::TooShort; return r; }
    if (get16(b, 0) != kStartCode) { r.error = ParseError::BadHeader; return r; }
    if (!valid_id(b[6])) { r.error = ParseError::InvalidIdentifier; return r; }
    const auto len = get16(b, 7);
    if (len < 2) { r.error = ParseError::InvalidLength; return r; }
    if (static_cast<std::size_t>(len - 2) > kMaxPayloadSize) { r.error = ParseError::Oversized; return r; }
    if (b.size() != kFixedPrefixSize + len) { r.error = ParseError::InvalidLength; return r; }
    r.packet.address = get32(b, 2); r.packet.id = static_cast<PacketId>(b[6]);
    r.packet.payload.assign(b.begin() + 9, b.end() - 2);
    if (get16(b, b.size() - 2) != checksum(r.packet.id, len, r.packet.payload)) r.error = ParseError::BadChecksum;
    return r;
}
}
