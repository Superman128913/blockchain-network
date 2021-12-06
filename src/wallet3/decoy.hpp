#pragma once

#include <crypto/crypto.h>
#include <cryptonote_basic/cryptonote_basic.h>
#include "output.hpp"

namespace wallet
{
  struct Decoy
  {
    //outs - array of structure outkey as follows:
    //height - unsigned int; block height of the output
    //key - String; the public key of the output
    //mask - String
    //txid - String; transaction id
    //unlocked - boolean; States if output is locked (false) or not (true)

    int64_t height;
    std::string key; // Hex public key of the output
    std::string mask; 
    std::string txid; 
    bool unlocked; 




  };
}  // namespace wallet
