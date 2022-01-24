#pragma once

#include <cryptonote_basic/cryptonote_basic.h>
#include "address.hpp"
#include "output.hpp"
#include "decoy.hpp"

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
    version tx_version;

    std::vector<TransactionRecipient> recipients;  // does not include change

    TransactionRecipient change;

    std::string memo;

    cryptonote::transaction tx;

    std::vector<Output> chosen_outputs;

    std::vector<std::vector<Decoy>> decoys;

    bool blink = true;

    int64_t fee = 0;
    uint64_t fee_per_byte = FEE_PER_BYTE_V13;
    uint64_t fee_per_output = FEE_PER_OUTPUT_V18;
    size_t mixin_count = CRYPTONOTE_DEFAULT_TX_MIXIN;
    size_t extra_size() const {return 0;};

    PendingTransaction() = default;

    PendingTransaction(const std::vector<TransactionRecipient>& new_recipients);

    int64_t
    get_fee() const;
    int64_t
    get_fee(int64_t n_inputs) const;

    size_t
    get_tx_weight(int64_t n_inputs) const;

    void
    update_change();

    int64_t
    sum_inputs() const;

    int64_t
    sum_outputs() const;

    bool
    finalise();
  };

}  // namespace wallet
