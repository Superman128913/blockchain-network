#pragma once

#include "transaction_scanner.hpp"
#include "transaction_constructor.hpp"
#include "daemon_comms.hpp"
#include "keyring.hpp"

#include <memory>
#include <string_view>

namespace db
{
  class Database;
}

namespace oxenmq
{
  class OxenMQ;
  class TimerID;
}

namespace wallet
{
  struct Block;

  class Wallet : public std::enable_shared_from_this<Wallet>
  {
   protected:
    Wallet(
        std::shared_ptr<oxenmq::OxenMQ> oxenMQ,
        std::shared_ptr<Keyring> keys,
        std::shared_ptr<TransactionConstructor> txConstructor,
        std::shared_ptr<DaemonComms> daemonComms,
        std::string_view dbFilename,
        std::string_view dbPassword);

    void
    init();

   public:
    template <typename... T>
    [[nodiscard]] static std::shared_ptr<Wallet>
    MakeWallet(T&&... args)
    {
      std::shared_ptr<Wallet> p{new Wallet(std::forward<T>(args)...)};
      p->init();
      return p;
    }

    ~Wallet();

    uint64_t
    GetBalance();
    uint64_t
    GetUnlockedBalance();
    address
    GetAddress();

    // FIXME: argument nomenclature
    address
    GetSubaddress(int32_t account, int32_t index);

    int64_t
    ScannedHeight();

    int64_t
    ScanTargetHeight();

    // TODO: error types to throw
    PendingTransaction
    CreateTransaction(
        const std::vector<std::pair<address, int64_t>>& recipients, int64_t feePerKB);
    void
    SignTransaction(PendingTransaction& tx);
    void
    SubmitTransaction(const PendingTransaction& tx);

    void
    AddBlock(const Block& block);

    void
    AddBlocks(const std::vector<Block>& blocks);

    // Called by daemon comms to inform of new sync target.
    void
    UpdateTopBlockInfo(int64_t height, const crypto::hash& hash);

    /* Tells the wallet to inform comms that it is going away.
     *
     * This MUST be called before the wallet is destroyed.
     */
    void
    Deregister();

   protected:
    void
    StoreTransaction(
        const crypto::hash& tx_hash, const int64_t height, const std::vector<Output>& outputs);

    void
    StoreSpends(
        const crypto::hash& tx_hash,
        const int64_t height,
        const std::vector<crypto::key_image>& spends);

    void
    RequestNextBlocks();

    std::shared_ptr<oxenmq::OxenMQ> oxenMQ;

    std::shared_ptr<db::Database> db;

    std::shared_ptr<Keyring> keys;
    TransactionScanner txScanner;
    std::shared_ptr<TransactionConstructor> txConstructor;
    std::shared_ptr<DaemonComms> daemonComms;

    int64_t scan_target_height = 0;
    int64_t last_scanned_height = -1;
    bool running = true;
  };

}  // namespace wallet
