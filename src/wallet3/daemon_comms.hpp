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
    SetRemote(std::string_view address) = 0;

    virtual int64_t
    GetHeight() = 0;

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
    RegisterWallet(Wallet& wallet, int64_t height, bool check_sync_height = false) = 0;

    virtual void
    DeregisterWallet(Wallet& wallet, std::promise<void>& p) = 0;
  };

}  // namespace wallet
