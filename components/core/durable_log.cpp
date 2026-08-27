#include "durable_log.hpp"
#include <fstream>
#include <regex>
#include <sstream>
namespace biosecure {
std::uint32_t crc32(const std::string& s){std::uint32_t c=0xffffffff;for(unsigned char b:s){c^=b;for(int i=0;i<8;i++)c=(c>>1)^(0xedb88320U&-(int(c&1)));}return ~c;}
bool DurableEventLog::append(const AttendanceEvent& e){auto json=e.canonicalJson();std::ofstream f(path_,std::ios::binary|std::ios::app);if(!f)return false;f<<std::hex<<crc32(json)<<'\t'<<json<<'\n';f.flush();return bool(f);}
std::vector<AttendanceEvent> DurableEventLog::recover(std::size_t* bad)const{std::vector<AttendanceEvent> out;std::ifstream f(path_,std::ios::binary);std::string line;std::size_t invalid=0;std::regex rx("^([0-9a-fA-F]+)\\t(\\{.*\\})$");std::regex fields(".*\\\"application_user_id\\\":\\\"([^\\\"]*)\\\".*\\\"device_id\\\":\\\"([^\\\"]*)\\\".*\\\"event_uuid\\\":\\\"([^\\\"]*)\\\".*\\\"firmware_version\\\":\\\"([^\\\"]*)\\\".*\\\"sensor_template_id\\\":([0-9]+).*\\\"sequence\\\":([0-9]+).*\\\"time_quality\\\":\\\"([^\\\"]*)\\\".*\\\"timestamp_ms\\\":(-?[0-9]+).*");std::smatch m;
while(std::getline(f,line)){std::smatch x;if(!std::regex_match(line,x,rx)){++invalid;continue;}std::uint32_t expected=0;std::stringstream(x[1].str())>>std::hex>>expected;auto json=x[2].str();if(expected!=crc32(json)||!std::regex_match(json,m,fields)){++invalid;continue;}AttendanceEvent e;e.application_user_id=m[1];e.device_id=m[2];e.event_uuid=m[3];e.firmware_version=m[4];e.sensor_template_id=std::uint16_t(std::stoul(m[5]));e.sequence=std::stoull(m[6]);auto q=m[7].str();e.time_quality=q=="synchronized"?TimeQuality::Synchronized:q=="estimated"?TimeQuality::Estimated:TimeQuality::Unknown;e.timestamp_ms=std::stoll(m[8]);e.result=AuthResult::Granted;out.push_back(e);}if(bad)*bad=invalid;return out;}
}
