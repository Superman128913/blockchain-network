#pragma once

#include <wallet3/default_daemon_comms.hpp>

namespace wallet
{

class MockDaemonComms: public DefaultDaemonComms
{
  public:

    MockDaemonComms() : DefaultDaemonComms(get_omq()){};

    std::shared_ptr<oxenmq::OxenMQ> get_omq() {
      return std::make_shared<oxenmq::OxenMQ>();
    }

    std::pair<int64_t, int64_t>
    get_fee_parameters() override { 
      return std::make_pair(0,0);
    }

    std::future<std::vector<Decoy>>
    fetch_decoys(std::vector<int64_t>& indexes) override {
      auto p = std::promise<std::vector<Decoy>>();
      auto fut = p.get_future();

      std::vector<Decoy> returned_decoys;
      for (auto index : indexes)
        returned_decoys.push_back(Decoy{index, "","","",true});

      p.set_value(returned_decoys);
      return fut;
    }
};


} // namespace wallet
