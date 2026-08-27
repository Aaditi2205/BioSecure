#include "state_machine.hpp"
namespace biosecure {
bool StateMachine::allowed(State f, State t) {
    if (f == t) return true;
    switch (f) {
    case State::Boot: return t == State::SelfTest;
    case State::SelfTest: return t==State::Idle||t==State::SensorError||t==State::StorageError||t==State::Offline;
    case State::Idle: return t==State::Capture||t==State::Enrollment||t==State::Syncing||t==State::Offline||t==State::Lockout||t==State::SensorError||t==State::StorageError;
    case State::Capture: return t==State::Processing||t==State::Denied||t==State::SensorError;
    case State::Processing: return t==State::Matched||t==State::Denied||t==State::SensorError;
    case State::Matched: return t==State::Logging;
    case State::Denied: return t==State::Idle||t==State::Lockout;
    case State::Enrollment: return t==State::Idle||t==State::SensorError||t==State::StorageError;
    case State::Logging: return t==State::Idle||t==State::StorageError;
    case State::Syncing: return t==State::Idle||t==State::Offline||t==State::NetworkError;
    case State::Offline: return t==State::Idle||t==State::Syncing||t==State::NetworkError;
    case State::Lockout: return t==State::Idle;
    case State::SensorError: return t==State::SelfTest||t==State::Idle;
    case State::StorageError: return t==State::SelfTest||t==State::Idle;
    case State::NetworkError: return t==State::Offline||t==State::Syncing||t==State::Idle;
    }
    return false;
}
bool StateMachine::transition(State n) { if (!allowed(state_,n)) return false; state_=n; return true; }
}
