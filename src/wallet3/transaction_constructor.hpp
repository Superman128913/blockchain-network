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
        : db(std::move(database)), daemon(std::move(dmn))
    {
      std::tie(fee_per_byte, fee_per_output) = daemon->get_fee_parameters();
    };

    PendingTransaction
    create_transaction(const std::vector<TransactionRecipient>& recipients) const;

    uint64_t fee_per_byte = FEE_PER_BYTE_V13;
    uint64_t fee_per_output = FEE_PER_OUTPUT_V18;

   private:
    void
    select_inputs(PendingTransaction& ptx) const;

    void
    select_and_fetch_decoys(PendingTransaction& ptx) const;

    void
    select_inputs_and_finalise(PendingTransaction& ptx) const;

    int64_t
    estimate_fee() const;

    std::shared_ptr<db::Database> db;
    std::shared_ptr<DaemonComms> daemon;

  };

}  // namespace wallet
