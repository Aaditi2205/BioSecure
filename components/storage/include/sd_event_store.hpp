#pragma once
#include "attendance.hpp"
#include <cstdio>
#include <string>
#include <vector>
namespace biosecure::storage {
struct SdConfig{int mosi{11};int miso{13};int sclk{12};int cs{10};std::string mount{"/sdcard"};};
class SdEventStore { public: explicit SdEventStore(SdConfig c):config_(std::move(c)){} ~SdEventStore(); bool mount(); bool append(const AttendanceEvent& event); bool acknowledge(const std::string& event_uuid); bool mounted()const{return mounted_;} std::vector<AttendanceEvent> pending()const; std::size_t pendingCount()const{return pending().size();} private:SdConfig config_;bool mounted_{false};void* card_{nullptr};};
}
