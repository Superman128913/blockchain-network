#pragma once

#include <cryptonote_basic/cryptonote_basic.h>
#include "address.hpp"
#include "output.hpp"

#include <vector>
#include <string>

namespace wallet
{
  struct version
  {};  // XXX: placeholder type

  struct TransactionRecipient
  {
    address recipient_address;
    int64_t amount;

    TransactionRecipient() = default;
    TransactionRecipient(address addr, int64_t amt) : recipient_address(addr), amount(amt){};
  };

  struct PendingTransaction
  {
    version txVersion;

    std::vector<TransactionRecipient> recipients;  // does not include change

    TransactionRecipient change;

    std::string memo;

    cryptonote::transaction tx;

    std::vector<Output> chosenOutputs;

    PendingTransaction() = default;

    PendingTransaction(const std::vector<TransactionRecipient>& new_recipients);

    void
    UpdateChange();

    int64_t
    SumInputs();

    int64_t
    SumOutputs();

    bool
    Finalise();
  };

}  // namespace wallet
