#include "mock_keyring.hpp"

#include <catch2/catch.hpp>

#include <wallet3/transaction_scanner.hpp>
#include <wallet3/block_tx.hpp>

#include <crypto/crypto.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include <cryptonote_basic/cryptonote_format_utils.h>

TEST_CASE("Transaction Scanner", "[wallet]")
{
  crypto::secret_key unused_secret_key;
  crypto::public_key tx_pubkey1;
  crypto::public_key tx_pubkey2;

  crypto::generate_keys(tx_pubkey1, unused_secret_key);
  crypto::generate_keys(tx_pubkey2, unused_secret_key);

  auto keys = std::make_shared<wallet::MockKeyring>();

  auto scanner = std::make_shared<wallet::TransactionScanner>(keys, nullptr);

  wallet::BlockTX block_tx;

  cryptonote::transaction& tx{block_tx.tx};

  cryptonote::tx_out out1{0,cryptonote::txout_to_key{tx_pubkey1}};

  cryptonote::tx_out out2{0,cryptonote::txout_to_key{tx_pubkey2}};

  SECTION("tx with no outputs created should yield no outputs for us")
  {
    REQUIRE(scanner->ScanTransactionReceived(block_tx, 0, 0).size() == 0);
  }

  cryptonote::add_tx_extra<cryptonote::tx_extra_pub_key>(tx, tx_pubkey1);

  tx.vout.push_back(out1);
  block_tx.global_indices.resize(1, 0);

  SECTION("tx has one output which is not ours")
  {
    REQUIRE(scanner->ScanTransactionReceived(block_tx, 0, 0).size() == 0);
  }

  SECTION("tx has one output which is ours")
  {
    keys->add_key_index_pair_as_ours(tx_pubkey1, 0, 0, {0,0});
    REQUIRE(scanner->ScanTransactionReceived(block_tx, 0, 0).size() == 1);
    REQUIRE(scanner->ScanTransactionReceived(block_tx, 0, 0)[0].subaddress_index == cryptonote::subaddress_index{0,0});
  }

  SECTION("subaddress_index is correct for identified output")
  {
    keys->add_key_index_pair_as_ours(tx_pubkey1, 0, 0, {1,0});
    REQUIRE(scanner->ScanTransactionReceived(block_tx, 0, 0).size() == 1);
    REQUIRE(scanner->ScanTransactionReceived(block_tx, 0, 0)[0].subaddress_index == cryptonote::subaddress_index{1,0});
  }

  SECTION("multiple outputs for multiple subaddresses")
  {
    keys->add_key_index_pair_as_ours(tx_pubkey1, 0, 0, {0,0});
    keys->add_key_index_pair_as_ours(tx_pubkey1, 1, 0, {3,4});
    tx.vout.push_back(out1); // second copy of same dummy output
    block_tx.global_indices.resize(2, 0);

    auto outs = scanner->ScanTransactionReceived(block_tx, 0, 0);
    REQUIRE(outs.size() == 2);
    REQUIRE(outs[0].subaddress_index == cryptonote::subaddress_index{0,0});
    REQUIRE(outs[1].subaddress_index == cryptonote::subaddress_index{3,4});
  }

  SECTION("some outputs for us, some not")
  {
    keys->add_key_index_pair_as_ours(tx_pubkey2, 1, 0, {0,0});
    tx.vout.push_back(out2); // diff output key, first not ours here, this one is
    block_tx.global_indices.resize(2, 0);

    auto outs = scanner->ScanTransactionReceived(block_tx, 0, 0);
    REQUIRE(outs.size() == 1);
    REQUIRE(outs[0].subaddress_index == cryptonote::subaddress_index{0,0});
  }

  SECTION("correct output amount")
  {
    keys->add_key_index_pair_as_ours(tx_pubkey1, 0, 42, {0,0});
    auto outs = scanner->ScanTransactionReceived(block_tx, 0, 0);
    REQUIRE(outs.size() == 1);
    REQUIRE(outs[0].amount == 42);
  }
}
