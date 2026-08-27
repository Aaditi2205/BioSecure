#include "sd_event_store.hpp"
#include "durable_log.hpp"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include <fstream>
#include <set>
namespace biosecure::storage {
SdEventStore::~SdEventStore(){if(mounted_)esp_vfs_fat_sdcard_unmount(config_.mount.c_str(),static_cast<sdmmc_card_t*>(card_));}
bool SdEventStore::mount(){sdmmc_host_t host=SDSPI_HOST_DEFAULT();spi_bus_config_t bus{};bus.mosi_io_num=config_.mosi;bus.miso_io_num=config_.miso;bus.sclk_io_num=config_.sclk;bus.quadwp_io_num=-1;bus.quadhd_io_num=-1;bus.max_transfer_sz=4096;if(spi_bus_initialize(static_cast<spi_host_device_t>(host.slot),&bus,SPI_DMA_CH_AUTO)!=ESP_OK)return false;sdspi_device_config_t slot=SDSPI_DEVICE_CONFIG_DEFAULT();slot.gpio_cs=static_cast<gpio_num_t>(config_.cs);slot.host_id=static_cast<spi_host_device_t>(host.slot);esp_vfs_fat_sdmmc_mount_config_t mount_cfg{};mount_cfg.format_if_mount_failed=false;mount_cfg.max_files=4;mount_cfg.allocation_unit_size=4096;sdmmc_card_t* card=nullptr;if(esp_vfs_fat_sdspi_mount(config_.mount.c_str(),&host,&slot,&mount_cfg,&card)!=ESP_OK){spi_bus_free(static_cast<spi_host_device_t>(host.slot));return false;}card_=card;mounted_=true;return true;}
bool SdEventStore::append(const AttendanceEvent& e){if(!mounted_)return false;DurableEventLog log(config_.mount+"/attendance.log");return log.append(e);}
bool SdEventStore::acknowledge(const std::string& id){if(!mounted_)return false;std::ofstream f(config_.mount+"/ack.log",std::ios::app|std::ios::binary);if(!f)return false;f<<id<<'\n';f.flush();return bool(f);}
std::vector<AttendanceEvent> SdEventStore::pending()const{if(!mounted_)return {};std::set<std::string> acked;std::ifstream a(config_.mount+"/ack.log",std::ios::binary);std::string id;while(std::getline(a,id))if(!id.empty())acked.insert(id);DurableEventLog log(config_.mount+"/attendance.log");auto events=log.recover();std::vector<AttendanceEvent> result;for(auto& event:events)if(!acked.count(event.event_uuid))result.push_back(std::move(event));return result;}
}
