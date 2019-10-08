#pragma once

#include "move.h"

#define TT_UNORDERED_MAP
#if   defined(TT_UNORDERED_MAP)
#include <unordered_map>
#elif defined(TT_KHASH)
// TODO: implement
#include "khash.h"
#else
#error "Must choose a hash table implementation"
#endif

namespace lesschess {

struct TT {
    enum class Flag : u8 {
        kExact,
        kLower,
        kUpper,
        kInvalid,
    };

    TT(size_t numEntries=(1u << 12))
    { table.reserve(numEntries); }

    struct Entry {

        bool is_valid() const noexcept
        { return flag != Flag::kInvalid; }

        bool is_exact() const noexcept
        { return flag == Flag::kExact; }

        bool is_lower() const noexcept
        { return flag == Flag::kLower; }

        bool is_upper() const noexcept
        { return flag == Flag::kUpper; }

        constexpr Entry() noexcept = default;

        constexpr Entry(Flag flag, int value, int depth) noexcept
            : flag{flag}, value{value}, depth{static_cast<u8>(depth)} {}

        Flag flag = Flag::kInvalid;
        u8   depth = 0;
        int  value = 0;
    };

    Entry& find(u64 hash) noexcept { return table[hash]; }

    void clear() noexcept { table.clear(); hits = 0; }

    void record_hit() noexcept { ++hits; }

    std::unordered_map<u64, Entry> table;
    s64 hits = 0;
};

} // ~namespace lesschess
