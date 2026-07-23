#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>

namespace kv {
using Clock = std::chrono::system_clock;
using TimePoint = Clock::time_point;

// constexpr makes this a compile-time constant
// inline lets compiler know any redefinition can be collapsed into 1 variable
inline constexpr std::size_t kNoSlot = SIZE_MAX;

struct Entry {
    std::string value;
    // nullopt = lives forever
    std::optional<TimePoint> expires_at;
    // Heap number with kNoSlot being a never expiring entry
    std::size_t heap_slot = kNoSlot;
};

}  // namespace kv