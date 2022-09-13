// Copyright (c) 2014-2019, The Monero Project
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without modification, are
// permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice, this list
//    of conditions and the following disclaimer in the documentation and/or other
//    materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its contributors may be
//    used to endorse or promote products derived from this software without specific
//    prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
// THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
// STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
// THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include "epee/readline_suspend.h"
#include "epee/misc_log_ex.h"
#include <iostream>
#include "logging/oxen_logger.h"
#include <fmt/color.h>

namespace tools
{

/************************************************************************/
/*                                                                      */
/************************************************************************/
class scoped_message_writer
{
private:
  bool m_flush;
  std::ostringstream m_oss;
  fmt::terminal_color m_color;
  oxen::log::Level m_log_level;
public:
  scoped_message_writer(
      fmt::terminal_color color = fmt::terminal_color::white 
    , std::string prefix = {}
    , spdlog::level::level_enum log_level = spdlog::level::info
    )
    : m_flush(true)
    , m_color(color)
    , m_log_level(log_level)
  {
    m_oss << prefix;
  }

  scoped_message_writer(scoped_message_writer&& rhs)
    : m_flush(std::move(rhs.m_flush))
    , m_oss(std::move(rhs.m_oss))
    , m_color(std::move(rhs.m_color))
    , m_log_level(std::move(rhs.m_log_level))
  {
    rhs.m_flush = false;
  }

  scoped_message_writer(scoped_message_writer& rhs) = delete;
  scoped_message_writer& operator=(scoped_message_writer& rhs) = delete;
  scoped_message_writer& operator=(scoped_message_writer&& rhs) = delete;

  template<typename T>
  std::ostream& operator<<(const T& val)
  {
    m_oss << val;
    return m_oss;
  }

  ~scoped_message_writer();
};

inline scoped_message_writer success_msg_writer(bool color = true)
{
  return scoped_message_writer(color ? fmt::terminal_color::green : fmt::terminal_color::white, std::string(), spdlog::level::info);
}

inline scoped_message_writer msg_writer(fmt::terminal_color color = fmt::terminal_color::white)
{
  return scoped_message_writer(color, std::string(), spdlog::level::info);
}

inline scoped_message_writer fail_msg_writer()
{
  return scoped_message_writer(fmt::terminal_color::red, "Error: ", spdlog::level::err);
}

} // namespace tools
