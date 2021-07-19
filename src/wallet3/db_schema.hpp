#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

namespace wallet
{
  void
  create_schema(SQLite::Database& db);
}
