#include "user_interface.hpp"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <array>
namespace biosecure::display {
namespace {constexpr i2c_port_t kPort=I2C_NUM_0;}
bool UserInterface::initialize(){i2c_config_t i{};i.mode=I2C_MODE_MASTER;i.sda_io_num=static_cast<gpio_num_t>(config_.sda);i.scl_io_num=static_cast<gpio_num_t>(config_.scl);i.sda_pullup_en=GPIO_PULLUP_ENABLE;i.scl_pullup_en=GPIO_PULLUP_ENABLE;i.master.clk_speed=400000;if(i2c_param_config(kPort,&i)!=ESP_OK||i2c_driver_install(kPort,i.mode,0,0,0)!=ESP_OK)return false;gpio_set_direction(static_cast<gpio_num_t>(config_.led),GPIO_MODE_OUTPUT);gpio_set_direction(static_cast<gpio_num_t>(config_.buzzer),GPIO_MODE_OUTPUT);for(auto c:{0xAE,0x20,0x00,0xB0,0xC8,0x00,0x10,0x40,0x81,0x7F,0xA1,0xA6,0xA8,0x3F,0xA4,0xD3,0x00,0xD5,0x80,0xD9,0xF1,0xDA,0x12,0xDB,0x40,0x8D,0x14,0xAF})command(static_cast<std::uint8_t>(c));return true;}
void UserInterface::command(std::uint8_t v){std::uint8_t bytes[2]={0x00,v};i2c_master_write_to_device(kPort,config_.oled_address,bytes,2,pdMS_TO_TICKS(20));}
void UserInterface::data(const std::uint8_t* b,std::size_t n){std::array<std::uint8_t,129> out{};out[0]=0x40;if(n>128)n=128;for(std::size_t i=0;i<n;i++)out[i+1]=b[i];i2c_master_write_to_device(kPort,config_.oled_address,out.data(),n+1,pdMS_TO_TICKS(20));}
void UserInterface::show(Feedback f){/* Compact state glyph: UI updates are bounded and never delay fingerprint work. */command(0x21);command(0);command(127);command(0x22);command(0);command(7);std::array<std::uint8_t,128> row{};auto code=static_cast<std::uint8_t>(f)+1;for(std::size_t i=0;i<row.size();i++)row[i]=(i%code)==0?0x7e:0x00;for(int page=0;page<8;page++)data(row.data(),row.size());bool success=f==Feedback::Granted||f==Feedback::Saved;gpio_set_level(static_cast<gpio_num_t>(config_.led),success);gpio_set_level(static_cast<gpio_num_t>(config_.buzzer),success);}
}
