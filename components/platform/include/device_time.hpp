#pragma once
#include "attendance.hpp"
#include <cstdint>
namespace biosecure::platform {
struct TimeReading{std::int64_t timestamp_ms;TimeQuality quality;};
class DeviceTime { public: void startSntp(); TimeReading now() const; bool synchronized() const; };
}
