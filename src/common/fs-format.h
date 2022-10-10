#pragma once

#include "fs.h"

#ifndef USE_GHC_FILESYSTEM
#include <fmt/std.h>
#else
#include "formattable.h"

namespace formattable {
  template <> inline constexpr bool via_to_string<ghc::filesystem::path> = true;

  inline std::string to_string(const ghc::filesystem::path& path) {
    return path.string();
  }
}
#endif
