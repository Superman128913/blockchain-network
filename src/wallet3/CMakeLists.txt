add_library(wallet3
  db_schema.cpp
  default_daemon_comms.cpp
  keyring.cpp
  transaction_constructor.cpp
  transaction_scanner.cpp
  pending_transaction.cpp
  output_selection/output_selection.cpp
  wallet.cpp
  wallet2½.cpp)

target_link_libraries(wallet3
  PRIVATE
  cryptonote_basic
  cryptonote_core
  SQLiteCpp)
