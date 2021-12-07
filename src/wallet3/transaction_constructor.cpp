#include "transaction_constructor.hpp"
#include "pending_transaction.hpp"
#include "output_selection/output_selection.hpp"
#include <sqlitedb/database.hpp>

namespace wallet
{
  PendingTransaction
  TransactionConstructor::create_transaction(
      const std::vector<TransactionRecipient>& recipients) const
  {
    PendingTransaction new_tx(recipients);
    new_tx.fee_per_byte = fee_per_byte;
    new_tx.fee_per_output = fee_per_output;
    select_inputs_and_finalise(new_tx);
    return new_tx;
  }


  // SelectInputs will choose some available unspent outputs from the database and allocate to the
  // transaction can be called multiple times and will add until enough is sufficient
  void
  TransactionConstructor::select_inputs(PendingTransaction& ptx) const
  {
    const int64_t single_input_size = ptx.get_fee(1);
    const int64_t double_input_size = ptx.get_fee(2);
    const int64_t additional_input = double_input_size - single_input_size;
    const int64_t dust_amount = single_input_size * ptx.fee_per_byte;

    OutputSelector select_outputs{};
    const int noutputs_estimate = 300;  // number of outputs to precompute fee for
    for (int64_t output_count = 1; output_count < noutputs_estimate; ++output_count)
    {
      select_outputs.push_fee(output_count, ptx.get_fee(output_count));
    }
    int64_t transaction_total = ptx.sum_outputs();

    // Check that we actually have enough in the outputs to build this transaction. Fail early. We
    // then increase the transaction_total to include an amount sufficient to cover a reasonable
    // change amount. Transaction fee is high for the first input because there is overhead to cover
    // and prefer that the change amount is enough to cover that overhead, but if we dont have enough
    // in the wallet then try to ensure there is enough to cover the fee
    // as an additional (2nd+) input. Finally if the wallet balance is not sufficient
    // allow the change to be dust but this will only occur if the wallet has enough to cover the
    // transaction but not enough to also cover the dust which should be extremely unlikely.
    int64_t wallet_balance = db->prepared_get<int>(
        "SELECT sum(amount) FROM outputs WHERE amount > ?", additional_input * static_cast<int64_t>(ptx.fee_per_byte));
    if (wallet_balance < transaction_total)
      throw std::runtime_error("Insufficient Wallet Balance");
    else if (wallet_balance > transaction_total + single_input_size * static_cast<int64_t>(ptx.fee_per_byte))
      transaction_total += single_input_size * ptx.fee_per_byte;
    else if (wallet_balance > transaction_total + additional_input * static_cast<int64_t>(ptx.fee_per_byte))
      transaction_total += additional_input * ptx.fee_per_byte;

    std::vector<Output> available_outputs{};
    SQLite::Statement st{
        db->db,
        "SELECT amount, output_index, global_index, unlock_time, block_height, spending, "
        "spent_height FROM outputs WHERE amount > ? ORDER BY amount"};
    st.bind(1, additional_input * static_cast<int64_t>(ptx.fee_per_byte));
    while (st.executeStep())
    {
      wallet::Output o(db::get<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>(st));
      available_outputs.push_back(o);
    }
    ptx.chosen_outputs = select_outputs(available_outputs, transaction_total);
    ptx.fee = ptx.get_fee();
    ptx.update_change();
  }

  void
  TransactionConstructor::select_inputs_and_finalise(PendingTransaction& ptx) const
  {
    while (true)
    {
      if (ptx.finalise())
        break;
      else
        select_inputs(ptx);
    }
  }
}  // namespace wallet
