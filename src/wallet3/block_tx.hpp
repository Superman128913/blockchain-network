#pragma once

#include <cryptonote_basic/cryptonote_basic.h>
#include <crypto/crypto.h>

namespace wallet
{
  struct BlockTX
  {
    crypto::hash hash;

    // output global indices to reference when spending
    std::vector<int64_t> global_indices;

    cryptonote::transaction tx;
  };

}  // namespace wallet
