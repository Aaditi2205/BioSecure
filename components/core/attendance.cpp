#include "attendance.hpp"
#include <iomanip>
#include <sstream>
namespace biosecure {
namespace { const char* aq(AuthResult r){return r==AuthResult::Granted?"granted":r==AuthResult::Denied?"denied":"duplicate";} const char* tq(TimeQuality q){return q==TimeQuality::Synchronized?"synchronized":q==TimeQuality::Estimated?"estimated":"unknown";} }
std::string AttendanceEvent::canonicalJson() const { std::ostringstream o; o<<"{\"application_user_id\":\""<<application_user_id<<"\",\"authentication_result\":\""<<aq(result)<<"\",\"device_id\":\""<<device_id<<"\",\"event_uuid\":\""<<event_uuid<<"\",\"firmware_version\":\""<<firmware_version<<"\",\"sensor_template_id\":"<<sensor_template_id<<",\"sequence\":"<<sequence<<",\"sync_state\":\"pending\",\"time_quality\":\""<<tq(time_quality)<<"\",\"timestamp_ms\":"<<timestamp_ms<<"}"; return o.str(); }
AttendanceService::AttendanceService(std::string d,std::string f,std::int64_t w,std::uint64_t s):device_(std::move(d)),firmware_(std::move(f)),window_(w),sequence_(s){}
std::string AttendanceService::uuid(std::uint64_t s,std::int64_t n) const { std::uint64_t x=1469598103934665603ULL; for(char c:device_) x=(x^std::uint8_t(c))*1099511628211ULL; x^=s+(std::uint64_t(n)<<1); std::ostringstream o;o<<std::hex<<std::setfill('0')<<std::setw(8)<<std::uint32_t(x>>32)<<"-"<<std::setw(4)<<std::uint16_t(x>>16)<<"-4"<<std::setw(3)<<std::uint16_t(x&0xfff)<<"-a"<<std::setw(3)<<std::uint16_t((x>>20)&0xfff)<<"-"<<std::setw(12)<<(x&0xffffffffffffULL);return o.str(); }
std::optional<AttendanceEvent> AttendanceService::authenticate(std::uint16_t id,const std::string& user,std::int64_t now,TimeQuality q,bool override_dup){auto it=last_seen_.find(id);if(!override_dup&&it!=last_seen_.end()&&now-it->second<=window_)return std::nullopt;last_seen_[id]=now;auto seq=++sequence_;return AttendanceEvent{uuid(seq,now),seq,device_,id,user,now,AuthResult::Granted,SyncState::Pending,q,firmware_};}
}
