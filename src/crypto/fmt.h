#pragma once

#include <fmt/format.h>
#include "crypto.h"
#include "hash.h"

template <>
struct fmt::formatter<crypto::public_key> : fmt::formatter<std::string> {
  auto format(crypto::public_key v, format_context& ctx) {
    return formatter<std::string>::format(
      fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::secret_key> : fmt::formatter<std::string> {
  auto format(crypto::secret_key v, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::key_derivation> : fmt::formatter<std::string> {
  auto format(crypto::key_derivation v, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::key_image> : fmt::formatter<std::string> {
  auto format(crypto::key_image v, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::signature> : fmt::formatter<std::string> {
  auto format(crypto::signature v, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::ed25519_public_key> : fmt::formatter<std::string> {
  auto format(crypto::ed25519_public_key v, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::x25519_public_key> : fmt::formatter<std::string> {
  auto format(crypto::x25519_public_key v, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(v)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::hash> : fmt::formatter<std::string> {
  auto format(crypto::hash h, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(h)), ctx);
  }
};
template <>
struct fmt::formatter<crypto::hash8> : fmt::formatter<std::string> {
  auto format(crypto::hash8 h, format_context& ctx) {
    return formatter<std::string>::format(
        fmt::format("<{}>", tools::type_to_hex(h)), ctx);
  }
};
