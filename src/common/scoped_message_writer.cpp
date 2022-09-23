#include "scoped_message_writer.h"
#include "common/format.h"

namespace tools {

static auto logcat = log::Cat("msgwriter");

scoped_message_writer::~scoped_message_writer()
{
  if (m_flush)
  {
    m_flush = false;
    if (fmt::terminal_color::white == m_color)
      logcat->log(m_log_level, m_oss.str());
    else
      logcat->log(m_log_level, "{}",
              log::detail::text_style_wrapper{fg(m_color), "{}", m_oss.str()});
    std::cout << std::endl;
  }
}

}
