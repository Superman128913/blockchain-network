#include "scoped_message_writer.h"

static auto logcat = oxen::log::Cat("msgwriter");

tools::scoped_message_writer::~scoped_message_writer()
{
  if (m_flush)
  {
    m_flush = false;
    if (fmt::terminal_color::white == m_color)
      logcat->log(m_log_level, m_oss.str());
    else
      logcat->log(m_log_level, fmt::format(fg(m_color),m_oss.str()));
    std::cout << std::endl;
  }
}
