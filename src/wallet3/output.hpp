#pragma once

#include <cstdint>
#include <string>

#include <crypto/crypto.h>
#include <cryptonote_basic/subaddress_index.h>
#include <ringct/rctTypes.h>

namespace wallet
{
  struct Output
  {
    int64_t amount;
    int64_t output_index;
    int64_t global_index;
    int64_t unlock_time;
    int64_t block_height;
    int64_t block_time;
    bool spending = false;
    int64_t spent_height;
    int64_t spent_time;

    crypto::hash tx_hash;
    crypto::public_key key;
    rct::key rct_mask;
    crypto::key_image key_image;
    cryptonote::subaddress_index subaddress_index;
  };

}  // namespace wallet
