#pragma once
#include "attendance.hpp"
#include <string>
#include <vector>
namespace biosecure { class DurableEventLog { public: explicit DurableEventLog(std::string path):path_(std::move(path)){} bool append(const AttendanceEvent& event); std::vector<AttendanceEvent> recover(std::size_t* invalid_records=nullptr) const; private:std::string path_;}; std::uint32_t crc32(const std::string& data); }
