#pragma once

#include <cstdbool>
#include <cstddef>
#include <cstdint>

#define u8 uint8_t
#define u32 uint32_t
#define u64 uint64_t

#define f64 double

// my life is ruined
// clang-format can't recognize trailing-return type with this macro
// you fucking donkeys
// so it formats into a stupid format
#define fnc auto
