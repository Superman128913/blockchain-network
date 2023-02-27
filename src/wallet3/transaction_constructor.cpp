#include "transaction_constructor.hpp"
#include "pending_transaction.hpp"
#include "decoy.hpp"
#include "output_selection/output_selection.hpp"
#include "decoy_selection/decoy_selection.hpp"
#include "db_schema.hpp"

#include <oxenc/base64.h>

#include <cryptonote_basic/hardfork.h>

//TODO: nettype-based tx construction parameters

namespace wallet
{
  // create_transaction will create a vanilla spend transaction without any special features.
  PendingTransaction
  TransactionConstructor::create_transaction(
      const std::vector<cryptonote::tx_destination_entry>& recipients,
      const cryptonote::tx_destination_entry& change_recipient)
  {
    PendingTransaction new_tx(recipients);
    auto [hf, hf_uint8] = cryptonote::get_ideal_block_version(db->network_type(), db->scan_target_height());
    cryptonote::oxen_construct_tx_params tx_params{hf, cryptonote::txtype::standard, 0, 0};
    new_tx.tx.version = cryptonote::transaction::get_max_version_for_hf(tx_params.hf_version);
    new_tx.tx.type = tx_params.tx_type;
    new_tx.fee_per_byte = fee_per_byte;
    new_tx.fee_per_output = fee_per_output;
    new_tx.change = change_recipient;
    select_inputs_and_finalise(new_tx);
    return new_tx;
  }

  PendingTransaction
  TransactionConstructor::create_ons_buy_transaction(
      const cryptonote::tx_destination_entry& change_recipient,
      const std::string& type_str,
      const std::string& owner_str,
      const std::string& backup_owner_str,
      const std::string& name,
      const std::string& value
      )
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    PendingTransaction new_tx(recipients);
    auto [hf, hf_uint8] = cryptonote::get_ideal_block_version(db->network_type(), db->scan_target_height());
    cryptonote::oxen_construct_tx_params tx_params{hf, cryptonote::txtype::oxen_name_system, 0, 0};
    new_tx.tx.version = cryptonote::transaction::get_max_version_for_hf(tx_params.hf_version);
    new_tx.tx.type = tx_params.tx_type;
    new_tx.fee_per_byte = fee_per_byte;
    new_tx.fee_per_output = fee_per_output;
    new_tx.change = change_recipient;
    new_tx.blink = false;

    std::string reason = "";

    const auto type = ons::parse_ons_type(type_str);
    if (!type.has_value())
      throw std::runtime_error("invalid type provided");

    const auto lower_name = tools::lowercase_ascii_string(name);
    if (!ons::validate_ons_name(*type, lower_name, &reason))
      throw std::runtime_error(reason);
    const auto name_hash = ons::name_to_hash(lower_name);

    ons::mapping_value encrypted_value;
    if (!ons::mapping_value::validate(nettype, *type, value, &encrypted_value, &reason))
      throw std::runtime_error(reason);

    if (!encrypted_value.encrypt(lower_name, &name_hash))
      throw std::runtime_error("Fail to encrypt mapping value=" + value);

    ons::generic_owner owner;
    ons::generic_owner backup_owner;

    if (owner_str == "")
      owner = ons::make_monero_owner(change_recipient.addr, change_recipient.is_subaddress);
    else if (!ons::parse_owner_to_generic_owner(nettype, owner_str, owner, &reason))
      throw std::runtime_error(reason);

    if (backup_owner_str != "" && !ons::parse_owner_to_generic_owner(nettype, backup_owner_str, backup_owner, &reason))
      throw std::runtime_error(reason);

    // No prev_txid for initial ons buy
    crypto::hash prev_txid = {};

    auto ons_buy_data = cryptonote::tx_extra_oxen_name_system::make_buy(
        owner,
        backup_owner_str != "" ? &backup_owner : nullptr,
        *type,
        name_hash,
        encrypted_value.to_string(),
        prev_txid);

    new_tx.burn_fixed = ons::burn_needed(cryptonote::get_latest_hard_fork(nettype).version, *type);
    new_tx.update_change();

    //Finally save the data to the extra field of our transaction
    cryptonote::add_oxen_name_system_to_tx_extra(new_tx.extra, ons_buy_data);
    cryptonote::add_burned_amount_to_tx_extra(new_tx.extra, new_tx.burn_fixed);

    select_inputs_and_finalise(new_tx);
    return new_tx;
  }

  PendingTransaction
  TransactionConstructor::create_ons_update_transaction(
      const cryptonote::tx_destination_entry& change_recipient,
      const std::string& type_str,
      const std::string& owner_str,
      const std::string& backup_owner_str,
      const std::string& name,
      const std::string& value,
      std::shared_ptr<Keyring> keyring
      )
  {
    if (value == "" && owner_str == "" && backup_owner_str == "")
      throw std::runtime_error("Value, owner and backup owner are not specified. Atleast one field must be specified for updating the ONS record");

    const auto lower_name = tools::lowercase_ascii_string(name);
    std::string reason;
    const auto type = ons::parse_ons_type(type_str);
    if (!type.has_value())
      throw std::runtime_error("invalid type provided");
    if (!ons::validate_ons_name(*type, lower_name, &reason))
      throw std::runtime_error(reason);
    const auto name_hash = ons::name_to_hash(lower_name);

    auto submit_ons_future = daemon->ons_names_to_owners(oxenc::to_base64(tools::view_guts(name_hash)), ons::db_mapping_type(*type));
    if (submit_ons_future.wait_for(5s) != std::future_status::ready)
      throw std::runtime_error("request to daemon for ons_names_to_owners timed out");

    const auto ons_response = submit_ons_future.get();
    crypto::hash prev_txid;
    std::string curr_owner;

    oxenc::bt_dict_consumer dc{ons_response};
    if (not dc.skip_until("owner"))
    {
      auto reason = dc.consume_string();
      throw std::runtime_error("Submit ons names to owners rejected, reason: " + reason);
    }
    curr_owner = dc.consume_string();

    if (not dc.skip_until("txid"))
    {
      auto reason = dc.consume_string();
      throw std::runtime_error("Submit ons names to owners rejected, reason: " + reason);
    }
    tools::hex_to_type<crypto::hash>(dc.consume_string(), prev_txid);

    ons::mapping_value encrypted_value;
    if (value != "")
    {
      if (!ons::mapping_value::validate(nettype, *type, value, &encrypted_value, &reason))
        throw std::runtime_error(reason);

      if (!encrypted_value.encrypt(lower_name, &name_hash))
        throw std::runtime_error("Fail to encrypt name");
    }

    ons::generic_owner owner;
    if (owner_str != "" && !ons::parse_owner_to_generic_owner(nettype, owner_str, owner, &reason))
      throw std::runtime_error(reason);

    ons::generic_owner backup_owner;
    if (backup_owner_str != "" && !ons::parse_owner_to_generic_owner(nettype, backup_owner_str, backup_owner, &reason))
      throw std::runtime_error(reason);

    const auto signature = keyring->generate_ons_signature(
      curr_owner,
      owner_str != "" ? &owner : nullptr,
      backup_owner_str != "" ? &backup_owner : nullptr,
      encrypted_value,
      prev_txid,
      nettype
      );

    std::vector<cryptonote::tx_destination_entry> recipients;
    PendingTransaction new_tx(recipients);
    auto [hf, hf_uint8] = cryptonote::get_ideal_block_version(db->network_type(), db->scan_target_height());
    cryptonote::oxen_construct_tx_params tx_params{hf, cryptonote::txtype::oxen_name_system, 0, 0};
    new_tx.tx.version = cryptonote::transaction::get_max_version_for_hf(tx_params.hf_version);
    new_tx.tx.type = tx_params.tx_type;
    new_tx.fee_per_byte = fee_per_byte;
    new_tx.fee_per_output = fee_per_output;
    new_tx.change = change_recipient;
    new_tx.blink = false;

    auto ons_update_data = cryptonote::tx_extra_oxen_name_system::make_update(
        signature,
        *type,
        name_hash,
        encrypted_value.to_string(),
        owner_str != "" ? &owner : nullptr,
        backup_owner_str != "" ? &backup_owner : nullptr,
        prev_txid);

    //Finally save the data to the extra field of our transaction
    cryptonote::add_oxen_name_system_to_tx_extra(new_tx.extra, ons_update_data);
    new_tx.update_change();

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
    int64_t wallet_balance = db->available_balance(additional_input * static_cast<int64_t>(ptx.fee_per_byte));
    if (wallet_balance < transaction_total)
      throw std::runtime_error("Insufficient Wallet Balance");
    else if (wallet_balance > transaction_total + single_input_size * static_cast<int64_t>(ptx.fee_per_byte))
      transaction_total += single_input_size * ptx.fee_per_byte;
    else if (wallet_balance > transaction_total + additional_input * static_cast<int64_t>(ptx.fee_per_byte))
      transaction_total += additional_input * ptx.fee_per_byte;

    // Selects all outputs where the amount is greater than the estimated fee for an ADDITIONAL input.
    auto available_outputs = db->available_outputs(additional_input * static_cast<int64_t>(ptx.fee_per_byte));
    ptx.chosen_outputs = select_outputs(available_outputs, ptx.sum_outputs());
    ptx.fee = ptx.get_fee();
    ptx.update_change();
  }

  // select_and_fetch_decoys will choose some available outputs from the database, fetch the
  // details necessary for a ring signature from the daemon and add them to the
  // transaction ready to sign at a later point in time.
  void
  TransactionConstructor::select_and_fetch_decoys(PendingTransaction& ptx)
  {
    ptx.decoys = {};
    // This initialises the decoys to be selected from global_output_index= 0 to global_output_index = highest_output_index
    int64_t max_output_index = db->chain_output_count();
    //DecoySelector decoy_selection(0, max_output_index);
    DecoySelector& decoy_selection = *decoy_selector;
    std::vector<int64_t> indexes;
    for (const auto& output : ptx.chosen_outputs)
    {
      indexes = decoy_selection(output);
      auto decoy_future = daemon->fetch_decoys(indexes);
      decoy_future.wait();
      ptx.decoys.emplace_back(decoy_future.get());

      bool good = false;
      for(const auto& decoy : ptx.decoys.back())
        good |= (output.key == decoy.key);
      if (!good)
        throw std::runtime_error{"Key from daemon for real output does not match our stored key."};
    }
  }

  void
  TransactionConstructor::select_inputs_and_finalise(PendingTransaction& ptx)
  {
    while (true)
    {
      if (ptx.finalise())
        break;
      else
        select_inputs(ptx);
    }
    select_and_fetch_decoys(ptx);
  }
}  // namespace wallet
