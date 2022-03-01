#include <filesystem>
#include <catch2/catch.hpp>

#include <wallet3/wallet.hpp>
#include <wallet3/db_schema.hpp>

#include <sqlitedb/database.hpp>

#include "mock_wallet.hpp"
#include "mock_keyring.hpp"
#include "mock_daemon_comms.hpp"


TEST_CASE("Transaction Creation", "[wallet,tx]")
{
  auto wallet = wallet::MockWallet();
  auto comms = std::make_shared<wallet::MockDaemonComms>();
  auto ctor = wallet::TransactionConstructor(wallet.get_db(), comms);
  ctor.fee_per_byte = 0;
  ctor.fee_per_output  = 0;
  SECTION("Expect Fail if database is empty")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 4;
    REQUIRE_THROWS(ctor.create_transaction(recipients));
  }

  wallet.store_test_transaction(5);

  SECTION("Creates a successful single transaction")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 4;
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 1);
    REQUIRE(ptx.change.amount == 1);
    REQUIRE(ptx.decoys.size() == ptx.chosen_outputs.size());
    for (const auto& decoys : ptx.decoys)
      REQUIRE(decoys.size() == 13);
  }

  SECTION("Fails to create a transaction if amount is not enough")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 6;
    REQUIRE_THROWS(ctor.create_transaction(recipients));
  }

  wallet.store_test_transaction(5);
  wallet.store_test_transaction(7);
  SECTION("Creates a successful single transaction prefering to use a single input if possible")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 6;
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 1);
    REQUIRE(ptx.change.amount == 1);
    REQUIRE(ptx.decoys.size() == ptx.chosen_outputs.size());
    for (const auto& decoys : ptx.decoys)
      REQUIRE(decoys.size() == 13);
  }

  SECTION("Creates a successful transaction using 2 inputs")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 8;
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 2);
    REQUIRE(ptx.decoys.size() == ptx.chosen_outputs.size());
    for (const auto& decoys : ptx.decoys)
      REQUIRE(decoys.size() == 13);
  }

  wallet.store_test_transaction(4000);
  wallet.store_test_transaction(4000);
  ctor.fee_per_byte = 1;

  SECTION("Creates a successful transaction using 2 inputs, avoids creating dust and uses correct fee using 1 oxen per byte")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 4001;
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 2);
    // 8000 (Inputs) - 4001 (Recipient) - 1857 bytes x 1 oxen (Fee)
    REQUIRE(ptx.change.amount == 2142);
    REQUIRE(ptx.decoys.size() == ptx.chosen_outputs.size());
    for (const auto& decoys : ptx.decoys)
      REQUIRE(decoys.size() == 13);
  }

  ctor.fee_per_output = 50;
  SECTION("Creates a successful transaction using 2 inputs, avoids creating dust and uses correct fee using 1 oxen per byte and 50 oxen per output")
  {
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 4001;
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 2);
    // 8000 (Inputs) - 4001 (Recipient) - 1857 bytes x 1 oxen (Fee) - 100 (Fee for 2x outputs @ 50 oxen) 
    REQUIRE(ptx.change.amount == 2042);
    REQUIRE(ptx.decoys.size() == ptx.chosen_outputs.size());
    for (const auto& decoys : ptx.decoys)
      REQUIRE(decoys.size() == 13);
  }

  SECTION("Creates a successful transaction then signs using the keyring successfully")
  {
    // Start a new wallet for real inputs to test signatures
    auto wallet_with_valid_inputs = wallet::MockWallet();
    auto ctor_for_signing = wallet::TransactionConstructor(wallet_with_valid_inputs.get_db(), comms);

    wallet::Output o{};

    wallet_with_valid_inputs.store_test_output(o);
    std::vector<cryptonote::tx_destination_entry> recipients;
    recipients.emplace_back(cryptonote::tx_destination_entry{});
    recipients.back().amount = 4001;
    wallet::PendingTransaction ptx = ctor_for_signing.create_transaction(recipients);
    REQUIRE(ptx.finalise());

    auto keys = std::make_unique<wallet::MockKeyring>();
    REQUIRE_NOTHROW(keys->sign_transaction(ptx));
    auto& signedtx = ptx.tx;
  }
}
