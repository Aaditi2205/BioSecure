#include "backoff.hpp"
#include <algorithm>
namespace biosecure { std::uint32_t ExponentialBackoff::next(){auto shift=std::min<std::uint32_t>(attempt_,30);std::uint64_t value=std::uint64_t(base_)<<shift;++attempt_;return static_cast<std::uint32_t>(std::min<std::uint64_t>(value,max_));} }
