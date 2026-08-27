#pragma once
#include <cstdint>
namespace biosecure {
enum class State : std::uint8_t { Boot, SelfTest, Idle, Capture, Processing, Matched, Denied, Enrollment, Logging, Syncing, Offline, Lockout, SensorError, StorageError, NetworkError };
class StateMachine { public: State state() const { return state_; } bool transition(State next); static bool allowed(State from, State to); private: State state_{State::Boot}; };
}
