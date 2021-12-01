#include <filesystem>
#include <catch2/catch.hpp>

#include <wallet3/wallet.hpp>
#include <wallet3/db_schema.hpp>

#include <sqlitedb/database.hpp>

#include "mock_wallet.hpp"
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
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 4);
    REQUIRE_THROWS(ctor.create_transaction(recipients));
  }

  wallet.store_test_transaction(5);

  SECTION("Creates a successful single transaction")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 4);
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 1);
    REQUIRE(ptx.change.amount == 1);
  }

  SECTION("Fails to create a transaction if amount is not enough")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 6);
    REQUIRE_THROWS(ctor.create_transaction(recipients));
  }

  wallet.store_test_transaction(5);
  wallet.store_test_transaction(7);
  SECTION("Creates a successful single transaction prefering to use a single input if possible")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 6);
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 1);
    REQUIRE(ptx.change.amount == 1);
  }

  SECTION("Creates a successful transaction using 2 inputs")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 8);
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 2);
  }

  wallet.store_test_transaction(4000);
  wallet.store_test_transaction(4000);
  ctor.fee_per_byte = 1;

  SECTION("Creates a successful transaction using 2 inputs, avoids creating dust and uses correct fee using 1 oxen per byte")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 4001);
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 2);
    // 8000 (Inputs) - 4001 (Recipient) - 1857 bytes x 1 oxen (Fee)
    REQUIRE(ptx.change.amount == 2142);
  }

  ctor.fee_per_output = 50;
  SECTION("Creates a successful transaction using 2 inputs, avoids creating dust and uses correct fee using 1 oxen per byte and 50 oxen per output")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 4001);
    wallet::PendingTransaction ptx = ctor.create_transaction(recipients);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosen_outputs.size() == 2);
    // 8000 (Inputs) - 4001 (Recipient) - 1857 bytes x 1 oxen (Fee) - 100 (Fee for 2x outputs @ 50 oxen) 
    REQUIRE(ptx.change.amount == 2042);
  }
}
