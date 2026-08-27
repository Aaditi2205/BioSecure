#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace biosecure::r307 {
constexpr std::uint16_t kStartCode = 0xEF01;
constexpr std::size_t kFixedPrefixSize = 9;
constexpr std::size_t kMaxPayloadSize = 256;
constexpr std::uint32_t kDefaultAddress = 0xFFFFFFFF;

enum class PacketId : std::uint8_t { Command = 0x01, Data = 0x02, Ack = 0x07, EndData = 0x08 };
enum class ParseError { None, TooShort, BadHeader, InvalidIdentifier, InvalidLength, Oversized, BadChecksum };

struct Packet {
    std::uint32_t address{kDefaultAddress};
    PacketId id{PacketId::Command};
    std::vector<std::uint8_t> payload;
};

struct ParseResult { Packet packet; ParseError error{ParseError::None}; };
std::uint16_t checksum(PacketId id, std::uint16_t wire_length, const std::vector<std::uint8_t>& payload);
std::vector<std::uint8_t> serialize(const Packet& packet);
ParseResult deserialize(const std::vector<std::uint8_t>& bytes);
}
