#pragma once

#include "daemon_comms.hpp"

#include <crypto/crypto.h>

#include <oxenmq/oxenmq.h>

#include <list>
#include <memory>

namespace wallet
{
  struct Wallet;
  struct Block;
  struct BlockTX;

  class DefaultDaemonComms : public DaemonComms, public std::enable_shared_from_this<DefaultDaemonComms>
  {
   private:
    static constexpr int64_t DEFAULT_MAX_RESPONSE_SIZE = 1 * 1024 * 1024; // 1 MiB
    static constexpr int64_t DEFAULT_MAX_SYNC_BLOCKS = 200;

    void
    OnGetBlocksResponse(std::vector<std::string> response);

    void
    RequestTopBlockInfo();

    void
    UpdateTopBlockInfo();

   public:

    DefaultDaemonComms(std::shared_ptr<oxenmq::OxenMQ> oxenMQ);

    void
    SetRemote(std::string_view address);

    int64_t
    GetHeight() { return top_block_height; }

    void
    RegisterWallet(wallet::Wallet& wallet, int64_t height, bool check_sync_height);

    void
    DeregisterWallet(Wallet& wallet, std::promise<void>& p);

   private:

    void
    ForEachWallet(std::function<void(std::shared_ptr<Wallet>)> func);

    void
    GetBlocks();

    void
    GotBlocks(int64_t start_height, int64_t end_height);

    void
    StartSyncing();

    std::unordered_map<std::shared_ptr<Wallet>, int64_t> wallets;

    std::shared_ptr<oxenmq::OxenMQ> oxenMQ;
    oxenmq::address remote;
    oxenmq::ConnectionID conn;
    oxenmq::TimerID status_timer;
    oxenmq::TaggedThreadID sync_thread;

    int64_t max_response_size = DEFAULT_MAX_RESPONSE_SIZE;

    crypto::hash top_block_hash;
    int64_t top_block_height = 0;

    int64_t sync_from_height = 0;
    bool syncing = false;
    int64_t max_sync_blocks = DEFAULT_MAX_SYNC_BLOCKS;
  };

}  // namespace wallet
