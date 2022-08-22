#include <wallet3/wallet.hpp>
#include <wallet3/default_daemon_comms.hpp>
#include <wallet3/keyring.hpp>
#include <wallet3/block.hpp>
#include <wallet3/block_tx.hpp>
#include <wallet3/wallet2½.hpp>
#include <wallet3/config/config.hpp>

#include <cryptonote_core/cryptonote_core.h>
#include <common/hex.h>
#include <oxenmq/oxenmq.h>

#include <atomic>
#include <thread>
#include <future>
#include <algorithm>

int main(int argc, char** argv)
{
  std::string wallet_name = "test_wallet1";
  crypto::secret_key spend_priv;
  crypto::public_key spend_pub;
  crypto::secret_key view_priv;
  crypto::public_key view_pub;
  std::string wallet_addr = "T6SYSC9FVpn15BGNpYYx3dHiATyjXoyqbSGBqgu5QbqEUmETnGSFqjtay42DBs6yZpVbgJcyhsbDUcUL3msN4GyW2HhR7aTmh";
  if (argc == 1 or std::string(argv[1]) == "1") // no cli arg = test wallet 1
  {
    tools::hex_to_type<crypto::secret_key>("d6a2eac72d1432fb816793aa7e8e86947116ac1423cbad5804ca49893e03b00c", spend_priv);
    tools::hex_to_type<crypto::public_key>("2fc259850413006e39450de23e3c63e69ccbdd3a14329707db55e3501bcda5fb", spend_pub);

    tools::hex_to_type<crypto::secret_key>("e93c833da9342958aff37c030cadcd04df8976c06aa2e0b83563205781cb8a02", view_priv);
    tools::hex_to_type<crypto::public_key>("5c1e8d44b4d7cb1269e69180dbf7aaf9c1fed4089b2bd4117dd1a70e90f19600", view_pub);
  }
  else // cli arg that isn't "1" = test wallet 2
  {
    wallet_name = "test_wallet2";
    wallet_addr = "T6ThzoXPCEvKjB9jce9rhd5gpovK5hvfX6yE1d6nC2H7QobLgoYNvW12qrahC9HtrtHNty58UXHZoNGyDnCeSFbY3Bu8yFtgh";
    tools::hex_to_type<crypto::secret_key>("e6c9165356c619a64a0d26fafd99891acccccf8717a8067859d972ecd8bcfc0a", spend_priv);
    tools::hex_to_type<crypto::public_key>("b76f2d7c8a036ff65c564dcb27081c04fe3f2157942e23b0496ca797ba728e4f", spend_pub);
    tools::hex_to_type<crypto::secret_key>("961d67bb5b3ed1af8678bbfcf621f9c15c2b7bff080892890020bdfd47fe4f0a", view_priv);
    tools::hex_to_type<crypto::public_key>("8a0ebacd613e0b03b8f27bc64bd961ea2ebf4c671c6e7f3268651acf0823fed5", view_pub);
  }

  std::cout << "Loading wallet \"" << wallet_name << "\" with address " << wallet_addr << "\n";

  auto keyring = std::make_shared<wallet::Keyring>(spend_priv, spend_pub, view_priv, view_pub);

  wallet::Config config;
  auto& comms_config = config.daemon;
  auto& omq_rpc_config = config.omq_rpc;
  auto oxenmq = std::make_shared<oxenmq::OxenMQ>();
  auto comms = std::make_shared<wallet::DefaultDaemonComms>(oxenmq, comms_config);
  config.omq_rpc.sockname = wallet_name + ".sock";
  auto wallet = wallet::Wallet::create(oxenmq, keyring, nullptr, comms, wallet_name + ".sqlite", "", config);

  std::this_thread::sleep_for(1s);
  auto chain_height = comms->get_height();

  std::cout << "chain height: " << chain_height << "\n";

  int64_t scan_height = -1;

  std::atomic<bool> done = false;

  std::thread exit_thread([&](){
      std::string foo;
      std::cin >> foo;
      done = true;
      });

  oxenmq::address remote{std::string("ipc://") + wallet_name + ".sock"};
  oxenmq::ConnectionID conn;
  conn = oxenmq->connect_remote(remote, [](auto){}, [](auto,auto){});

  while (not done)
  {
    using namespace std::chrono_literals;

    chain_height = comms->get_height();
    std::cout << "chain height: " << chain_height << "\n";
    scan_height = wallet->last_scan_height;
    std::cout << "after block " << scan_height << ", " << wallet_name << " balance is: " << wallet->get_balance() << "\n";
    std::this_thread::sleep_for(2s);
  }

  /*
  oxenmq::bt_dict req;
  oxenmq::bt_list dests;
  oxenmq::bt_dict d;
  d["address"] = wallet_addr2;
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
  */

  std::cout << "scanning appears finished, " << wallet_name << " scan height = " << wallet->last_scan_height << ", daemon comms height = " << comms->get_height() << "\n";

  wallet->deregister();
}
