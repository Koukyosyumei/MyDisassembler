#pragma once
#include <tuple>
#include <unordered_map>
#include <string>

namespace std {
namespace {

// Code from boost
// Reciprocal of the golden ratio helps spread entropy
//     and handles duplicates.
// See Mike Seymour in magic-numbers-in-boosthash-combine:
//     http://stackoverflow.com/questions/4948780

template <class T>
inline std::size_t hash_combine(std::size_t seed, T const& v) {
    return seed ^= std::hash<T>()(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Recursive template code derived from Matthieu M.
template <class Tuple, size_t Index = std::tuple_size<Tuple>::value - 1>
struct HashValueImpl {
    static void apply(size_t &seed, Tuple const& tuple) {
        HashValueImpl<Tuple, Index - 1>::apply(seed, tuple);
        seed = hash_combine(seed, std::get<Index>(tuple));
    }
};

template <class Tuple>
struct HashValueImpl<Tuple, 0> {
    static void apply(size_t &seed, Tuple const& tuple) {
        seed = hash_combine(seed, std::get<0>(tuple));
    }
};
}  // namespace

template <class T, class S>
struct hash<std::pair<T, S>> {
    size_t operator()(const std::pair<T, S>& keyval) const noexcept {
        return hash_combine<S>(std::hash<T>()(keyval.first), keyval.second);
    }
};

template <typename... TT>
struct hash<std::tuple<TT...>> {
    size_t operator()(std::tuple<TT...> const& tt) const {
        size_t seed = 0;
        HashValueImpl<std::tuple<TT...>>::apply(seed, tt);
        return seed;
    }
};
}  // namespace std

inline int u2d(const std::string& utf8_str) {
    int result = 0;
    for (unsigned char c : utf8_str) {
        result = (result << 8) + c;
    }
    return result;
}

