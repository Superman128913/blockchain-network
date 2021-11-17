#include "transaction_constructor.hpp"
#include "pending_transaction.hpp"

namespace wallet
{
  PendingTransaction::PendingTransaction(const std::vector<TransactionRecipient>& new_recipients)
      : recipients(new_recipients)
  {
    // TODO sean address of change needs to be creator
    change = TransactionRecipient{{}, 0};
    int64_t sum_recipient_amounts = 0;
    for (const auto& recipient : new_recipients)
    {
      if (sum_recipient_amounts < 0 || recipient.amount < 0)
        throw std::runtime_error("Transaction amounts must be positive");
      sum_recipient_amounts += recipient.amount;
    }
    if (new_recipients.empty() || sum_recipient_amounts < 0)
      throw std::runtime_error("Transaction amounts must be positive");
  }

  void
  PendingTransaction::UpdateChange()
  {
    change.amount = SumInputs() - SumOutputs();
  }

  int64_t
  PendingTransaction::SumInputs()
  {
    return std::accumulate(
        chosenOutputs.begin(),
        chosenOutputs.end(),
        0,
        [](int64_t accumulator, const Output& output) { return accumulator + output.amount; });
  }

  int64_t
  PendingTransaction::SumOutputs()
  {
    return std::accumulate(
        recipients.begin(),
        recipients.end(),
        0,
        [](int64_t accumulator, const TransactionRecipient& recipient) {
          return accumulator + recipient.amount;
        });
  }

  bool
  PendingTransaction::Finalise()
  {
    if (SumInputs() - SumOutputs() - change.amount == 0)
      return true;
    else
      return false;
  }

}  // namespace wallet
