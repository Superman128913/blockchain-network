#pragma once

#include "output.hpp"
#include "keyring.hpp"

#include <cryptonote_basic/cryptonote_basic.h>

#include <vector>

namespace db
{
  class Database;
}

namespace wallet
{
  struct BlockTX;

  class TransactionScanner
  {
   public:
    TransactionScanner(std::shared_ptr<Keyring> _keys, std::shared_ptr<db::Database> _db)
        : wallet_keys(_keys), db(_db)
    {}

    std::vector<Output>
    ScanTransactionReceived(const BlockTX& tx, int64_t height, int64_t timestamp);

    std::vector<crypto::key_image>
    ScanTransactionSpent(const cryptonote::transaction& tx);

   private:
    std::shared_ptr<Keyring> wallet_keys;
    std::shared_ptr<db::Database> db;
  };

}  // namespace wallet
