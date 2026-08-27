#pragma once
#include <cstdint>
#include <map>
#include <optional>
#include <string>
namespace biosecure {
enum class AuthResult { Granted, Denied, Duplicate };
enum class SyncState { Pending, Acknowledged };
enum class TimeQuality { Synchronized, Estimated, Unknown };
struct AttendanceEvent { std::string event_uuid; std::uint64_t sequence{}; std::string device_id; std::uint16_t sensor_template_id{}; std::string application_user_id; std::int64_t timestamp_ms{}; AuthResult result{AuthResult::Denied}; SyncState sync{SyncState::Pending}; TimeQuality time_quality{TimeQuality::Unknown}; std::string firmware_version; std::string canonicalJson() const; };
class AttendanceService {
public: AttendanceService(std::string device, std::string firmware, std::int64_t duplicate_window_ms=45000, std::uint64_t initial_sequence=0);
    std::optional<AttendanceEvent> authenticate(std::uint16_t template_id, const std::string& user_id, std::int64_t now_ms, TimeQuality quality, bool override_duplicate=false);
private: std::string uuid(std::uint64_t sequence, std::int64_t now) const; std::string device_; std::string firmware_; std::int64_t window_; std::uint64_t sequence_; std::map<std::uint16_t,std::int64_t> last_seen_;
};
}
