#include "device_time.hpp"
#include "esp_netif_sntp.h"
#include "esp_timer.h"
#include <ctime>
namespace biosecure::platform {
void DeviceTime::startSntp(){esp_sntp_config_t config=ESP_NETIF_SNTP_DEFAULT_CONFIG("pool.ntp.org");esp_netif_sntp_init(&config);}
bool DeviceTime::synchronized()const{return esp_netif_sntp_sync_wait(0)==ESP_OK;}
TimeReading DeviceTime::now()const{std::time_t seconds=0;std::time(&seconds);if(synchronized())return {static_cast<std::int64_t>(seconds)*1000,TimeQuality::Synchronized};return {esp_timer_get_time()/1000,TimeQuality::Unknown};}
}
