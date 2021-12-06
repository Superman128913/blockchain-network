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
        std::shared_ptr<oxenmq::OxenMQ> omq,
        std::shared_ptr<Keyring> keys,
        std::shared_ptr<TransactionConstructor> tx_constructor,
        std::shared_ptr<DaemonComms> daemon_comms,
        std::string_view dbFilename,
        std::string_view dbPassword);

    void
    init();

   public:
    template <typename... T>
    [[nodiscard]] static std::shared_ptr<Wallet>
    create(T&&... args)
    {
      std::shared_ptr<Wallet> p{new Wallet(std::forward<T>(args)...)};
      p->init();
      return p;
    }

    ~Wallet();

    uint64_t
    get_balance();
    uint64_t
    get_unlocked_balance();
    address
    get_address();

    // FIXME: argument nomenclature
    address
    get_subaddress(int32_t account, int32_t index);

    // TODO: error types to throw
    PendingTransaction
    create_transaction(
        const std::vector<std::pair<address, int64_t>>& recipients, int64_t feePerKB);
    void
    sign_transaction(PendingTransaction& tx);
    void
    submit_transaction(const PendingTransaction& tx);

    void
    add_block(const Block& block);

    void
    add_blocks(const std::vector<Block>& blocks);

    // Called by daemon comms to inform of new sync target.
    void
    update_top_block_info(int64_t height, const crypto::hash& hash);

    /* Tells the wallet to inform comms that it is going away.
     *
     * This MUST be called before the wallet is destroyed.
     */
    void
    deregister();

    int64_t scan_target_height = 0;
    int64_t last_scanned_height = -1;

   protected:
    void
    store_transaction(
        const crypto::hash& tx_hash, const int64_t height, const std::vector<Output>& outputs);

    void
    store_spends(
        const crypto::hash& tx_hash,
        const int64_t height,
        const std::vector<crypto::key_image>& spends);

    std::shared_ptr<oxenmq::OxenMQ> omq;

    std::shared_ptr<db::Database> db;

    std::shared_ptr<Keyring> keys;
    TransactionScanner tx_scanner;
    std::shared_ptr<TransactionConstructor> tx_constructor;
    std::shared_ptr<DaemonComms> daemon_comms;
    bool running = true;
  };

}  // namespace wallet
