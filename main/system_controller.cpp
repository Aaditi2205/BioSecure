#include "system_controller.hpp"
#ifdef ESP_PLATFORM
#include "attendance.hpp"
#include "device_time.hpp"
#include "esp_log.h"
#include "esp_task_wdt.h"
#include "esp_uart_transport.hpp"
#include "network_client.hpp"
#include "r307_driver.hpp"
#include "sd_event_store.hpp"
#include "user_interface.hpp"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include <cstring>

namespace biosecure { namespace {
constexpr char kTag[]="BioSecure"; constexpr std::size_t kDepth=16; constexpr std::uint16_t kSearchCapacity=1000;
enum EventBits:EventBits_t{WifiUp=BIT0,TimeSynced=BIT1,StorageReady=BIT2};
enum class Kind:std::uint8_t{Match,Denied,Persist,Submit,Ui};
struct Message{Kind kind{};std::uint16_t template_id{};std::int64_t timestamp_ms{};TimeQuality quality{TimeQuality::Unknown};display::Feedback feedback{display::Feedback::Ready};char uuid[37]{};char user[64]{};std::uint64_t sequence{};};
struct Runtime{QueueHandle_t attendance{},storage{},network{},ui{};EventGroupHandle_t events{};platform::EspUartTransport uart{{UART_NUM_1,17,18,57600,1024}};r307::Driver sensor{uart};platform::DeviceTime clock;AttendanceService service{CONFIG_BIOSECURE_DEVICE_ID,"0.1.0",CONFIG_BIOSECURE_DUPLICATE_WINDOW_MS};storage::SdEventStore store{{11,13,12,10,"/sdcard"}};networking::NetworkClient net{{CONFIG_BIOSECURE_WIFI_SSID,CONFIG_BIOSECURE_WIFI_PASSWORD,CONFIG_BIOSECURE_ATTENDANCE_URL,"",5000}};display::UserInterface display{{8,9,4,5,0x3C}};std::uint32_t failures{0};TickType_t locked_until{0};};
Runtime* runtime=nullptr;
void send_ui(display::Feedback f){Message m{};m.kind=Kind::Ui;m.feedback=f;if(xQueueSend(runtime->ui,&m,0)!=pdTRUE)ESP_LOGW(kTag,"UI queue full");}
AttendanceEvent event_from(const Message& m){return {m.uuid,m.sequence,CONFIG_BIOSECURE_DEVICE_ID,m.template_id,m.user,m.timestamp_ms,AuthResult::Granted,SyncState::Pending,m.quality,"0.1.0"};}
void fingerprint_task(void*){esp_task_wdt_add(nullptr);for(;;){if(runtime->locked_until&&xTaskGetTickCount()<runtime->locked_until){vTaskDelay(pdMS_TO_TICKS(250));esp_task_wdt_reset();continue;}send_ui(display::Feedback::PlaceFinger);auto capture=runtime->sensor.captureImage();if(capture.error!=r307::DriverError::None){send_ui(display::Feedback::SensorUnavailable);vTaskDelay(pdMS_TO_TICKS(1000));esp_task_wdt_reset();continue;}if(capture.confirmation==r307::Confirmation::NoFinger){vTaskDelay(pdMS_TO_TICKS(100));esp_task_wdt_reset();continue;}send_ui(display::Feedback::Reading);auto converted=runtime->sensor.imageToTemplate(1);auto match=converted.ok()?runtime->sensor.searchTemplate(1,0,kSearchCapacity):r307::OperationResult{};Message m{};if(match.ok()&&match.data.size()>=4){m.kind=Kind::Match;m.template_id=(std::uint16_t(match.data[0])<<8)|match.data[1];auto now=runtime->clock.now();m.timestamp_ms=now.timestamp_ms;m.quality=now.quality;runtime->failures=0;}else{m.kind=Kind::Denied;if(++runtime->failures>=CONFIG_BIOSECURE_MAX_FAILED_ATTEMPTS){runtime->locked_until=xTaskGetTickCount()+pdMS_TO_TICKS(CONFIG_BIOSECURE_LOCKOUT_MS);runtime->failures=0;}send_ui(display::Feedback::Unknown);}if(xQueueSend(runtime->attendance,&m,pdMS_TO_TICKS(20))!=pdTRUE)ESP_LOGW(kTag,"attendance queue full");vTaskDelay(pdMS_TO_TICKS(300));esp_task_wdt_reset();}}
void attendance_task(void*){esp_task_wdt_add(nullptr);Message m{};for(;;){if(xQueueReceive(runtime->attendance,&m,pdMS_TO_TICKS(250))==pdTRUE&&m.kind==Kind::Match){auto event=runtime->service.authenticate(m.template_id,"",m.timestamp_ms,m.quality);if(!event)send_ui(display::Feedback::Duplicate);else{Message out{};out.kind=Kind::Persist;out.template_id=event->sensor_template_id;out.timestamp_ms=event->timestamp_ms;out.quality=event->time_quality;out.sequence=event->sequence;std::strncpy(out.uuid,event->event_uuid.c_str(),sizeof(out.uuid)-1);if(xQueueSend(runtime->storage,&out,pdMS_TO_TICKS(20))!=pdTRUE)send_ui(display::Feedback::StorageUnavailable);}}esp_task_wdt_reset();}}
void storage_task(void*){esp_task_wdt_add(nullptr);if(runtime->store.mount()){xEventGroupSetBits(runtime->events,StorageReady);for(const auto& event:runtime->store.pending()){Message recovered{};recovered.kind=Kind::Submit;recovered.template_id=event.sensor_template_id;recovered.timestamp_ms=event.timestamp_ms;recovered.quality=event.time_quality;recovered.sequence=event.sequence;std::strncpy(recovered.uuid,event.event_uuid.c_str(),sizeof(recovered.uuid)-1);if(xQueueSend(runtime->network,&recovered,0)!=pdTRUE)break;}}else send_ui(display::Feedback::StorageUnavailable);Message m{};for(;;){if(xQueueReceive(runtime->storage,&m,pdMS_TO_TICKS(250))==pdTRUE&&m.kind==Kind::Persist){if(runtime->store.append(event_from(m))){send_ui(display::Feedback::Granted);send_ui(display::Feedback::Saved);m.kind=Kind::Submit;if(xQueueSend(runtime->network,&m,0)!=pdTRUE)ESP_LOGW(kTag,"network queue full; event remains durable");}else send_ui(display::Feedback::StorageUnavailable);}esp_task_wdt_reset();}}
void network_task(void*){esp_task_wdt_add(nullptr);if(runtime->net.initialize())runtime->clock.startSntp();Message m{};for(;;){if(xQueueReceive(runtime->network,&m,pdMS_TO_TICKS(250))==pdTRUE&&m.kind==Kind::Submit){send_ui(display::Feedback::Synchronizing);if(runtime->net.submit(event_from(m))==networking::SubmitResult::Acknowledged)runtime->store.acknowledge(m.uuid);else{send_ui(display::Feedback::Offline);vTaskDelay(pdMS_TO_TICKS(runtime->net.nextRetryMs()));if(xQueueSendToFront(runtime->network,&m,0)!=pdTRUE)ESP_LOGW(kTag,"retry queue full; durable event retained");}}esp_task_wdt_reset();}}
void ui_task(void*){esp_task_wdt_add(nullptr);runtime->display.initialize();Message m{};for(;;){if(xQueueReceive(runtime->ui,&m,pdMS_TO_TICKS(250))==pdTRUE)runtime->display.show(m.feedback);esp_task_wdt_reset();}}
bool task(TaskFunction_t f,const char* n,std::uint32_t stack,UBaseType_t priority){return xTaskCreate(f,n,stack,nullptr,priority,nullptr)==pdPASS;}
}
void SystemController::start(){static Runtime r;runtime=&r;r.events=xEventGroupCreate();r.attendance=xQueueCreate(kDepth,sizeof(Message));r.storage=xQueueCreate(kDepth,sizeof(Message));r.network=xQueueCreate(kDepth,sizeof(Message));r.ui=xQueueCreate(kDepth,sizeof(Message));if(!r.events||!r.attendance||!r.storage||!r.network||!r.ui||!r.uart.initialize()){ESP_LOGE(kTag,"initialization failed");return;}if(!r.sensor.verifyPassword(0).ok())ESP_LOGW(kTag,"R307 unavailable; task will keep recovering");if(!task(fingerprint_task,"FingerprintTask",4096,6)||!task(attendance_task,"AttendanceTask",4096,5)||!task(storage_task,"StorageTask",5120,4)||!task(network_task,"NetworkTask",6144,3)||!task(ui_task,"UserInterfaceTask",4096,2))ESP_LOGE(kTag,"task creation failed");}
}
#else
namespace biosecure {void SystemController::start(){}}
#endif
