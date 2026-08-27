#pragma once
#include <cstdint>
#include <string_view>
namespace biosecure::display {
enum class Feedback:std::uint8_t{Ready,PlaceFinger,Reading,Granted,Unknown,Duplicate,Offline,Saved,Synchronizing,SensorUnavailable,StorageUnavailable,Enrollment};
struct UiConfig{int sda{8};int scl{9};int led{4};int buzzer{5};std::uint8_t oled_address{0x3C};};
class UserInterface{public:explicit UserInterface(UiConfig c):config_(c){}bool initialize();void show(Feedback feedback);private:void command(std::uint8_t value);void data(const std::uint8_t* bytes,std::size_t size);UiConfig config_;};
}
