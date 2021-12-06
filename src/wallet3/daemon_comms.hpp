#pragma once

#include "decoy.hpp"
#include <memory>
#include <future>

namespace wallet
{
  class Wallet;
  class Block;

  class DaemonComms
  {
   public:

    virtual void
    set_remote(std::string_view address) = 0;

    virtual int64_t
    get_height() = 0;

    /* Called by a wallet to tell the daemon comms it exists, as well
     * as what height it needs to sync from.  Updates existing registration
     * if the wallet is already registered.
     *
     * A wallet should call this:
     *   On creation, to inform the daemon comms that it exists and wishes to sync.
     *   If the wallet recieves blocks from daemon comms which are in the future for
     *   it.  In this case, it is telling the daemon comms to start syncing from
     *   earlier in the chain.  Pass check_sync_height=true.
     *   When the wallet finishes processing a batch of blocks.  Pass check_sync_height=false.
     */
    virtual void
    register_wallet(Wallet& wallet, int64_t height, bool check_sync_height = false) = 0;

    virtual void
    deregister_wallet(Wallet& wallet, std::promise<void>& p) = 0;

    virtual std::pair<int64_t, int64_t>
    get_fee_parameters() = 0;

    virtual std::future<std::vector<Decoy>>
    fetch_decoys(std::vector<int64_t>& indexes) = 0;
  };

}  // namespace wallet
