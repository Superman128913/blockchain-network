#pragma once

#include <fmt/format.h>
#include "common/format.h"
#include "crypto.h"
#include "hash.h"

namespace crypto {
    template <typename T, typename SFINAE = void>
    constexpr bool is_hex_printable = false;

    template <typename T>
    constexpr bool is_hex_printable<T, std::enable_if_t<std::is_base_of_v<ec_point, T> && sizeof(T) == sizeof(ec_point)>> = true;

    template <> inline constexpr bool is_hex_printable<signature> = true;
    template <> inline constexpr bool is_hex_printable<ed25519_public_key> = true;
    template <> inline constexpr bool is_hex_printable<x25519_public_key> = true;
    template <> inline constexpr bool is_hex_printable<hash> = true;
    template <> inline constexpr bool is_hex_printable<hash8> = true;

    template <typename T> inline constexpr bool is_hex_printable<T&> = is_hex_printable<T>;
    template <typename T> inline constexpr bool is_hex_printable<T&&> = is_hex_printable<T>;
    template <typename T> inline constexpr bool is_hex_printable<const T> = is_hex_printable<T>;

    // Helper for when you are really sure you want to print a secret key (which is not printable by
    // default so that you have to be explicit and can't accidentally expose one in a log
    // statement).
    inline std::string expose_secret(const ec_scalar& secret) {
        return "<{}>"_format(tools::type_to_hex(secret));
    }
}

template <typename T, typename Char>
struct fmt::formatter<T, Char, std::enable_if_t<crypto::is_hex_printable<T>>> : fmt::formatter<std::string> {
  auto format(const T& v, format_context& ctx) {
    return formatter<std::string>::format("<{}>"_format(tools::type_to_hex(v)), ctx);
  }
};
