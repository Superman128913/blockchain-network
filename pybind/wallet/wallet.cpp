#include "../common.hpp"

#include <wallet3/wallet.hpp>
#include <wallet3/default_daemon_comms.hpp>
#include <wallet3/keyring.hpp>
#include <wallet3/config/config.hpp>
#include <oxenmq/oxenmq.h>

namespace wallet
{
  void
  Wallet_Init(py::module& mod)
  {
    py::class_<Wallet, std::shared_ptr<Wallet>>(mod, "Wallet")
      .def(py::init([](const std::string& wallet_name, std::shared_ptr<Keyring> keyring, Config config) {
        auto& comms_config = config.daemon;
        auto& omq_rpc_config = config.omq_rpc;
        auto oxenmq = std::make_shared<oxenmq::OxenMQ>();
        auto comms = std::make_shared<DefaultDaemonComms>(std::move(oxenmq), comms_config);
        return Wallet::create(oxenmq, std::move(keyring), nullptr, std::move(comms), wallet_name + ".sqlite", "", std::move(config));
      }))
      .def("get_balance", &Wallet::get_balance)
      .def("get_unlocked_balance", &Wallet::get_unlocked_balance)
      .def("deregister", &Wallet::deregister);
  }

}  // namespace wallet
