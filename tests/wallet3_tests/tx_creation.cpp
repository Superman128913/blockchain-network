#include <filesystem>
#include <catch2/catch.hpp>

#include <wallet3/wallet.hpp>
#include <wallet3/db_schema.hpp>
#include <wallet3/default_daemon_comms.hpp>

#include <sqlitedb/database.hpp>

#include "mock_wallet.hpp"


TEST_CASE("Transaction Creation", "[wallet,tx]")
{
  auto wallet = wallet::MockWallet();
  auto ctor = wallet::TransactionConstructor(wallet.GetDB(), nullptr);
  SECTION("Expect Fail if database is empty")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 4);
    REQUIRE_THROWS(ctor.CreateTransaction(recipients, {}));
  }

  wallet.StoreTestTransaction(5);

  SECTION("Creates a successful single transaction")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 4);
    wallet::PendingTransaction ptx = ctor.CreateTransaction(recipients, {});
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosenOutputs.size() == 1);
    REQUIRE(ptx.change.amount == 1);
  }

  SECTION("Fails to create a transaction if amount is not enough")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 6);
    REQUIRE_THROWS(ctor.CreateTransaction(recipients, {}));
  }

  wallet.StoreTestTransaction(5);
  wallet.StoreTestTransaction(7);
  SECTION("Creates a successful single transaction prefering to use a single input if possible")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 6);
    wallet::PendingTransaction ptx = ctor.CreateTransaction(recipients, {});
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosenOutputs.size() == 1);
    REQUIRE(ptx.change.amount == 1);
  }

  SECTION("Creates a successful transaction using 2 inputs")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 8);
    wallet::PendingTransaction ptx = ctor.CreateTransaction(recipients, {});
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosenOutputs.size() == 2);
  }

  wallet.StoreTestTransaction(1000);
  wallet.StoreTestTransaction(1000);

  SECTION("Creates a successful transaction using 2 inputs and avoids creating dust")
  {
    std::vector<wallet::TransactionRecipient> recipients;
    recipients.emplace_back(wallet::address{}, 1001);
    wallet::PendingTransaction ptx = ctor.CreateTransaction(recipients, 1000);
    REQUIRE(ptx.recipients.size() == 1);
    REQUIRE(ptx.chosenOutputs.size() == 2);
    REQUIRE(ptx.change.amount == 999);
  }
}
