#pragma once

#include <filesystem>

#include <oxen/log.hpp>
#include <oxenmq/oxenmq.h>

#define OXEN_LOG_ENABLED(LVL) logcat->should_log(spdlog::level::LVL)

inline auto globallogcat = oxen::log::Cat("global");

namespace oxen::logging
{
  void
  init(const std::string& log_location, oxen::log::Level log_level);
  void
  set_additional_log_categories(const oxen::log::Level& log_level);
  void
  process_categories_string(const std::string& categories);

  std::optional<oxen::log::Level>
  parse_level(std::string_view input);
  std::optional<oxen::log::Level>
  parse_level(uint8_t input);
  std::optional<oxen::log::Level>
  parse_level(oxenmq::LogLevel input);

}  // namespace oxen::logging
