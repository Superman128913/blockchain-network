#include "transaction_constructor.hpp"
#include "pending_transaction.hpp"
#include "output_selection/output_selection.hpp"
#include <sqlitedb/database.hpp>

namespace wallet
{
  PendingTransaction
  TransactionConstructor::CreateTransaction(
      const std::vector<TransactionRecipient>& recipients, int64_t feePerKB) const
  {
    PendingTransaction txNew(recipients);
    SelectInputsAndFinalise(txNew, feePerKB);
    return txNew;
  }

  // SelectInputs will choose some available unspent outputs from the database and allocate to the
  // transaction can be called multiple times and will add until enough is sufficient
  void
  TransactionConstructor::SelectInputs(PendingTransaction& ptx, int64_t feePerKB) const
  {
    const int64_t single_input_size = 1500;
    const int64_t additional_input = 500;
    // const int64_t feePerKB = 0.000366 * 1e9;
    const int64_t dust_amount = single_input_size * feePerKB / 1000;
    int64_t estimated_fee = EstimateFee();
    // int64_t estimated_fee = estimate_fee(2, fake_outs_count, min_outputs, extra.size(), clsag,
    // base_fee, fee_percent, fixed_fee,     fee_quantization_mask);
    int64_t transaction_total = ptx.SumOutputs() + estimated_fee;

    // Check that we actually have enough in the outputs to build this transaction. Fail early. We
    // then increase the transaction_total to include an amount sufficient to cover a reasonable
    // change amount. Transaction fee is high for the first input and prefer that the change amount
    // is enough to cover that, but if we dont have enough in the wallet then try for enough to
    // cover the fee as an additional (2nd+) input. Finally if the wallet balance is not sufficient
    // allow the change to be dust but this will only occur if the wallet has enough to cover the
    // transaction but not enough to also cover the dust which should be extremely unlikely.
    int64_t wallet_balance = db->prepared_get<int>(
        "SELECT sum(amount) FROM outputs WHERE amount > ?", additional_input * feePerKB / 1000);
    if (wallet_balance < transaction_total)
      throw std::runtime_error("Insufficient Wallet Balance");
    else if (wallet_balance > transaction_total + single_input_size * feePerKB / 1000)
      transaction_total += single_input_size * feePerKB / 1000;
    else if (wallet_balance > transaction_total + additional_input * feePerKB / 1000)
      transaction_total += additional_input * feePerKB / 1000;

    std::vector<Output> available_outputs{};
    SQLite::Statement st{
        db->db,
        "SELECT amount, output_index, global_index, unlock_time, block_height, spending, "
        "spent_height FROM outputs WHERE amount > ? ORDER BY amount"};
    st.bind(1, additional_input * feePerKB / 1000);
    while (st.executeStep())
    {
      wallet::Output o(db::get<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>(st));
      available_outputs.push_back(o);
    }
    OutputSelector selectOutputs{};
    ptx.chosenOutputs = selectOutputs(available_outputs, transaction_total);
    ptx.UpdateChange();
  }

  void
  TransactionConstructor::SelectInputsAndFinalise(PendingTransaction& ptx, int64_t feePerKB) const
  {
    while (true)
    {
      if (ptx.Finalise())
        break;
      else
        SelectInputs(ptx, feePerKB);
    }
  }

  int64_t
  TransactionConstructor::EstimateFee() const
  {
    return 0;
  }

}  // namespace wallet
