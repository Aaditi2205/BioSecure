#pragma once
#include <cstdint>
namespace biosecure { class ExponentialBackoff { public: ExponentialBackoff(std::uint32_t base_ms=1000,std::uint32_t max_ms=60000):base_(base_ms),max_(max_ms){} std::uint32_t next(); void reset(){attempt_=0;} std::uint32_t attempts()const{return attempt_;} private:std::uint32_t base_,max_,attempt_{0};}; }
