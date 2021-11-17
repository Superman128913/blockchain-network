#pragma once

#include <wallet3/wallet.hpp>
#include <wallet3/block.hpp>
#include <sqlitedb/database.hpp>

namespace wallet
{

template <typename T>
T debug_random_filled(uint64_t seed) {
    static_assert(sizeof(T) % 8 == 0 && alignof(T) >= alignof(uint64_t)
        && std::is_trivially_copyable_v<T>);
    T value;
    auto* value_u64 = reinterpret_cast<uint64_t*>(&value);
    std::mt19937_64 rng{seed};
    for (size_t i = 0; i < sizeof(T) / sizeof(uint64_t); i++)
        value_u64[i] = rng();
    return value;
}

class MockWallet : public Wallet
{
  public:

    MockWallet() : Wallet({},{},{},{},":memory:",{}){};

    int64_t height = 0;

    std::shared_ptr<db::Database> GetDB() { return db; };

    void
    StoreTestTransaction(const int64_t amount) 
    {
      height++;

      wallet::Block b{};
      b.height = height;
      auto hash = debug_random_filled<crypto::hash>(height);
      b.hash = hash;
      AddBlock(b);

      std::vector<wallet::Output> dummy_outputs;
      wallet::Output o{};
      o.amount = amount;
      o.block_height = height;
      o.subaddress_index = cryptonote::subaddress_index{0,0};
      o.key_image = debug_random_filled<crypto::key_image>(height);
      dummy_outputs.push_back(o);

      SQLite::Transaction db_tx(db->db);
      StoreTransaction(hash, height, dummy_outputs);
      db_tx.commit();
    };
};


} // namespace wallet
