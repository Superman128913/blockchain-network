#pragma once

#include <cstdint>
#include <vector>
#include <memory>
#include "address.hpp"
#include "pending_transaction.hpp"
#include "daemon_comms.hpp"

namespace db
{
  class Database;
}

namespace wallet
{
  class TransactionConstructor
  {
   public:
    TransactionConstructor(std::shared_ptr<db::Database> database, std::shared_ptr<DaemonComms> dmn)
        : db(std::move(database)), daemon(std::move(dmn)){};

    PendingTransaction
    CreateTransaction(const std::vector<TransactionRecipient>& recipients, int64_t feePerKB) const;

   private:
    void
    SelectInputs(PendingTransaction& ptx, int64_t feePerKB) const;
    void
    SelectInputsAndFinalise(PendingTransaction& ptx, int64_t feePerKB) const;
    int64_t
    EstimateFee() const;

    std::shared_ptr<db::Database> db;
    std::shared_ptr<DaemonComms> daemon;
  };

}  // namespace wallet
