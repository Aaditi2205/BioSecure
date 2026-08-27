#pragma once
#include "attendance.hpp"
#include "backoff.hpp"
#include <cstdint>
#include <string>
namespace biosecure::networking {
struct NetworkConfig{std::string ssid;std::string password;std::string attendance_url;std::string bearer_token;std::uint32_t request_timeout_ms{5000};};
enum class SubmitResult{Acknowledged,Offline,Timeout,Rejected,InvalidResponse,TlsError};
class NetworkClient { public: explicit NetworkClient(NetworkConfig c):config_(std::move(c)){} bool initialize(); bool connected()const; SubmitResult submit(const AttendanceEvent& event); std::uint32_t nextRetryMs(){return backoff_.next();} void resetBackoff(){backoff_.reset();} private:NetworkConfig config_;ExponentialBackoff backoff_;};
}
