#include "keyring.hpp"

#include "wallet2½.hpp"

#include <stdexcept>

namespace wallet
{
  crypto::key_derivation
  Keyring::generate_key_derivation(const crypto::public_key& tx_pubkey) const
  {
    return crypto::generate_key_derivation(tx_pubkey, view_private_key);
  }

  std::vector<crypto::key_derivation>
  Keyring::generate_key_derivations(const std::vector<crypto::public_key>& tx_pubkeys) const
  {
    std::vector<crypto::key_derivation> derivations;
    for (const auto& key : tx_pubkeys)
    {
      derivations.push_back(generate_key_derivation(key));
    }
    return derivations;
  }

  crypto::public_key
  Keyring::output_spend_key(
      const crypto::key_derivation& derivation,
      const crypto::public_key& output_key,
      uint64_t output_index)
  {
    crypto::public_key ret;

    // bool return here is pretty meaningless, but allocate it anyway to not discard its existence
    // entirely
    bool r = key_device.derive_subaddress_public_key(output_key, derivation, output_index, ret);

    return ret;
  }

  std::optional<cryptonote::subaddress_index>
  Keyring::output_and_derivation_ours(
      const crypto::key_derivation& derivation,
      const crypto::public_key& output_key,
      uint64_t output_index)
  {
    auto candidate_key = output_spend_key(derivation, output_key, output_index);

    // TODO: handle checking against subaddresses
    if (candidate_key == spend_public_key)
    {
      return cryptonote::subaddress_index{0, 0};
    }
    return std::nullopt;
  }

  crypto::key_image
  Keyring::key_image(
      const crypto::key_derivation& derivation,
      const crypto::public_key& output_key,
      uint64_t output_index,
      const cryptonote::subaddress_index& sub_index)
  {
    // TODO: subaddress support, for now throw if not main address
    if (not sub_index.is_zero())
    {
      throw std::invalid_argument("Subaddresses not yet supported in wallet3");
    }

    crypto::secret_key output_private_key;

    // computes Hs(a*R || idx) + b
    key_device.derive_secret_key(derivation, output_index, spend_private_key, output_private_key);

    crypto::public_key output_pubkey_computed;
    key_device.secret_key_to_public_key(output_private_key, output_pubkey_computed);

    // confirm derived output public key matches the output key in the transaction
    if (output_key != output_pubkey_computed)
    {
      throw std::invalid_argument("Output public key does not match derived output key.");
    }

    crypto::key_image ret;
    key_device.generate_key_image(output_key, output_private_key, ret);
    return ret;
  }

  // TODO: replace later when removing wallet2½ layer
  uint64_t
  Keyring::output_amount(
      const rct::rctSig& rv,
      const crypto::key_derivation& derivation,
      unsigned int i,
      rct::key& mask)
  {
    return wallet2½::output_amount(rv, derivation, i, mask, key_device);
  }

}  // namespace wallet
