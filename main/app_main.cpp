#include "system_controller.hpp"
#ifdef ESP_PLATFORM
#include "nvs_flash.h"
extern "C" void app_main(){nvs_flash_init();static biosecure::SystemController controller;controller.start();}
#endif
