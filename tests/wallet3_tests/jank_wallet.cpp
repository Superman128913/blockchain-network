#include <cryptonote_core/cryptonote_core.h>
#include <wallet3/wallet.hpp>
#include <wallet3/default_daemon_comms.hpp>
#include <wallet3/keyring.hpp>
#include <wallet3/block.hpp>
#include <wallet3/block_tx.hpp>
#include <wallet3/wallet2½.hpp>
#include <common/hex.h>
#include <oxenmq/oxenmq.h>

#include <atomic>
#include <thread>
#include <future>

int main(void)
{
  // after block 719259, balance is: 16109908470850

  crypto::secret_key spend_priv;
  tools::hex_to_type<crypto::secret_key>("d6a2eac72d1432fb816793aa7e8e86947116ac1423cbad5804ca49893e03b00c", spend_priv);
  crypto::public_key spend_pub;
  tools::hex_to_type<crypto::public_key>("2fc259850413006e39450de23e3c63e69ccbdd3a14329707db55e3501bcda5fb", spend_pub);

  crypto::secret_key view_priv;
  tools::hex_to_type<crypto::secret_key>("e93c833da9342958aff37c030cadcd04df8976c06aa2e0b83563205781cb8a02", view_priv);
  crypto::public_key view_pub;
  tools::hex_to_type<crypto::public_key>("5c1e8d44b4d7cb1269e69180dbf7aaf9c1fed4089b2bd4117dd1a70e90f19600", view_pub);

  std::string wallet_addr = "T6SYSC9FVpn15BGNpYYx3dHiATyjXoyqbSGBqgu5QbqEUmETnGSFqjtay42DBs6yZpVbgJcyhsbDUcUL3msN4GyW2HhR7aTmh";

  auto keyring = std::make_shared<wallet::Keyring>(spend_priv, spend_pub, view_priv, view_pub);

  auto oxenmq = std::make_shared<oxenmq::OxenMQ>();
  auto comms = std::make_shared<wallet::DefaultDaemonComms>(oxenmq);
  cryptonote::address_parse_info senders_address{};
  cryptonote::get_account_address_from_str(senders_address, cryptonote::network_type::TESTNET, wallet_addr);
  auto wallet = wallet::Wallet::create(oxenmq, keyring, nullptr, comms, "test.sqlite", "");

  std::this_thread::sleep_for(1s);
  auto chain_height = comms->get_height();

  std::cout << "chain height: " << chain_height << "\n";

  int64_t scan_height = 0;

  std::atomic<bool> done = false;

  std::thread exit_thread([&](){
      std::string foo;
      std::cin >> foo;
      done = true;
      });

  while (chain_height == 0 or (scan_height != chain_height and chain_height != 0))
  {
    using namespace std::chrono_literals;

    chain_height = comms->get_height();
    std::cout << "chain height: " << chain_height << "\n";
    scan_height = wallet->last_scan_height;
    std::this_thread::sleep_for(2s);
    std::cout << "after block " << scan_height << ", balance is: " << wallet->get_balance() << "\n";
    if (done)
      break;
  }

  oxenmq::address remote{"ipc://rpc.sock"};
  oxenmq::ConnectionID conn;
  conn = oxenmq->connect_remote(remote, [](auto){}, [](auto,auto){});

  oxenmq::bt_dict req;
  oxenmq::bt_list dests;
  oxenmq::bt_dict d;
  d["address"] = wallet_addr;
  d["amount"] = 4206980085;
  dests.push_back(std::move(d));
  req["destinations"] = std::move(dests);

  std::promise<bool> p;
  auto f = p.get_future();
  auto req_cb = [&p](bool ok, std::vector<std::string> response) mutable
      {
        std::cout << "transfer response, bool ok = " << std::boolalpha << ok << "\n";
        size_t n = 0;
        for (const auto& s : response)
        {
          std::cout << "response string " << n++ << ": " << s << "\n";
        }
        p.set_value(ok);
      };

  //oxenmq->request(conn, "rpc.get_height", req_cb, "de");
  oxenmq->request(conn, "restricted.transfer", req_cb, oxenmq::bt_serialize(req));

  f.wait();
  exit_thread.join();

  std::cout << "scanning appears finished, scan height = " << wallet->last_scan_height << ", daemon comms height = " << comms->get_height() << "\n";

  wallet->deregister();
}
