#include "core_rpc_server_commands_defs.h"
#include <nlohmann/json.hpp>
#include <oxenmq/base64.h>

namespace cryptonote::rpc {

void RPC_COMMAND::set_bt() {
  bt = true;
  response_b64.format = json_binary_proxy::fmt::bt;
  response_hex.format = json_binary_proxy::fmt::bt;
}

void to_json(nlohmann::json& j, const block_header_response& h)
{
  j = nlohmann::json
  {
    {"major_version", h.major_version},
    {"minor_version", h.minor_version},
    {"timestamp", h.timestamp},
    {"prev_hash", h.prev_hash},
    {"nonce", h.nonce},
    {"orphan_status", h.orphan_status},
    {"height", h.height},
    {"depth", h.depth},
    {"hash", h.hash},
    {"difficulty", h.difficulty},
    {"cumulative_difficulty", h.cumulative_difficulty},
    {"reward", h.reward},
    {"miner_reward", h.miner_reward},
    {"block_size", h.block_size},
    {"block_weight", h.block_weight},
    {"num_txes", h.num_txes},
    {"pow_hash", h.pow_hash ? *h.pow_hash : nullptr},
    {"long_term_weight", h.long_term_weight},
    {"miner_tx_hash", h.miner_tx_hash},
    {"miner_tx_hash", h.miner_tx_hash},
    {"tx_hashes", h.tx_hashes},
    {"service_node_winner", h.service_node_winner},
  };
};

void from_json(const nlohmann::json& j, block_header_response& h)
{
  j.at("major_version").get_to(h.major_version);
  j.at("minor_version").get_to(h.minor_version);
  j.at("timestamp").get_to(h.timestamp);
  j.at("prev_hash").get_to(h.prev_hash);
  j.at("nonce").get_to(h.nonce);
  j.at("orphan_status").get_to(h.orphan_status);
  j.at("height").get_to(h.height);
  j.at("depth").get_to(h.depth);
  j.at("hash").get_to(h.hash);
  j.at("difficulty").get_to(h.difficulty);
  j.at("cumulative_difficulty").get_to(h.cumulative_difficulty);
  j.at("reward").get_to(h.reward);
  j.at("miner_reward").get_to(h.miner_reward);
  j.at("block_size").get_to(h.block_size);
  j.at("block_weight").get_to(h.block_weight);
  j.at("num_txes").get_to(h.num_txes);
  if (j.at("pow_hash").is_null())
    h.pow_hash = std::nullopt;
  else
    h.pow_hash = j["pow_hash"].get<std::string>();
  j.at("long_term_weight").get_to(h.long_term_weight);
  j.at("miner_tx_hash").get_to(h.miner_tx_hash);
  j.at("miner_tx_hash").get_to(h.miner_tx_hash);
  j.at("tx_hashes").get_to(h.tx_hashes);
  j.at("service_node_winner").get_to(h.service_node_winner);
}

KV_SERIALIZE_MAP_CODE_BEGIN(STATUS)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(EMPTY)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(block_header_response)
  KV_SERIALIZE(major_version)
  KV_SERIALIZE(minor_version)
  KV_SERIALIZE(timestamp)
  KV_SERIALIZE(prev_hash)
  KV_SERIALIZE(nonce)
  KV_SERIALIZE(orphan_status)
  KV_SERIALIZE(height)
  KV_SERIALIZE(depth)
  KV_SERIALIZE(hash)
  KV_SERIALIZE(difficulty)
  KV_SERIALIZE(cumulative_difficulty)
  KV_SERIALIZE(reward)
  KV_SERIALIZE(miner_reward)
  KV_SERIALIZE(block_size)
  KV_SERIALIZE_OPT(block_weight, (uint64_t)0)
  KV_SERIALIZE(num_txes)
  KV_SERIALIZE(pow_hash)
  KV_SERIALIZE_OPT(long_term_weight, (uint64_t)0)
  KV_SERIALIZE(miner_tx_hash)
  KV_SERIALIZE(tx_hashes)
  KV_SERIALIZE(service_node_winner)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_BLOCK_HEADER_BY_HEIGHT::request)
  KV_SERIALIZE(height)
  KV_SERIALIZE(heights)
  KV_SERIALIZE_OPT(fill_pow_hash, false);
  KV_SERIALIZE_OPT(get_tx_hashes, false);
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_BLOCK_HEADER_BY_HEIGHT::response)
  KV_SERIALIZE(block_header)
  KV_SERIALIZE(block_headers)
  KV_SERIALIZE(status)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_BLOCK::request)
  KV_SERIALIZE(hash)
  KV_SERIALIZE(height)
  KV_SERIALIZE_OPT(fill_pow_hash, false);
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_BLOCK::response)
  KV_SERIALIZE(block_header)
  KV_SERIALIZE(tx_hashes)
  KV_SERIALIZE(status)
  KV_SERIALIZE(blob)
  KV_SERIALIZE(json)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_BLOCK_HEADERS_RANGE::request)
  KV_SERIALIZE(start_height)
  KV_SERIALIZE(end_height)
  KV_SERIALIZE_OPT(fill_pow_hash, false);
  KV_SERIALIZE_OPT(get_tx_hashes, false);
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_BLOCK_HEADERS_RANGE::response)
  KV_SERIALIZE(status)
  KV_SERIALIZE(headers)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()

KV_SERIALIZE_MAP_CODE_BEGIN(SET_BOOTSTRAP_DAEMON::request)
  KV_SERIALIZE(address)
  KV_SERIALIZE(username)
  KV_SERIALIZE(password)
KV_SERIALIZE_MAP_CODE_END()

KV_SERIALIZE_MAP_CODE_BEGIN(GET_OUTPUT_HISTOGRAM::request)
  KV_SERIALIZE(amounts);
  KV_SERIALIZE(min_count);
  KV_SERIALIZE(max_count);
  KV_SERIALIZE(unlocked);
  KV_SERIALIZE(recent_cutoff);
KV_SERIALIZE_MAP_CODE_END()

KV_SERIALIZE_MAP_CODE_BEGIN(GET_OUTPUT_HISTOGRAM::entry)
  KV_SERIALIZE(amount);
  KV_SERIALIZE(total_instances);
  KV_SERIALIZE(unlocked_instances);
  KV_SERIALIZE(recent_instances);
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_OUTPUT_HISTOGRAM::response)
  KV_SERIALIZE(status)
  KV_SERIALIZE(histogram)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_ALTERNATE_CHAINS::chain_info)
  KV_SERIALIZE(block_hash)
  KV_SERIALIZE(height)
  KV_SERIALIZE(length)
  KV_SERIALIZE(difficulty)
  KV_SERIALIZE(block_hashes)
  KV_SERIALIZE(main_chain_parent_block)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_ALTERNATE_CHAINS::response)
  KV_SERIALIZE(status)
  KV_SERIALIZE(chains)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(RELAY_TX::request)
  KV_SERIALIZE(txids)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_OUTPUT_DISTRIBUTION::request)
  KV_SERIALIZE(amounts)
  KV_SERIALIZE_OPT(from_height, (uint64_t)0)
  KV_SERIALIZE_OPT(to_height, (uint64_t)0)
  KV_SERIALIZE_OPT(cumulative, false)
  KV_SERIALIZE_OPT(binary, true)
  KV_SERIALIZE_OPT(compress, false)
KV_SERIALIZE_MAP_CODE_END()


namespace
{
  template<typename T>
  std::string compress_integer_array(const std::vector<T> &v)
  {
    std::string s;
    s.reserve(tools::VARINT_MAX_LENGTH<T>);
    auto ins = std::back_inserter(s);
    for (const T &t: v)
      tools::write_varint(ins, t);
    return s;
  }

  template<typename T>
  std::vector<T> decompress_integer_array(const std::string &s)
  {
    std::vector<T> v;
    for (auto it = s.begin(); it < s.end(); )
    {
      int read = tools::read_varint(it, s.end(), v.emplace_back());
      CHECK_AND_ASSERT_THROW_MES(read > 0, "Error decompressing data");
    }
    return v;
  }
}

KV_SERIALIZE_MAP_CODE_BEGIN(GET_OUTPUT_DISTRIBUTION::distribution)
  KV_SERIALIZE(amount)
  KV_SERIALIZE_N(data.start_height, "start_height")
  KV_SERIALIZE(binary)
  KV_SERIALIZE(compress)
  if (binary)
  {
    if (is_store)
    {
      if (compress)
      {
        const_cast<std::string&>(compressed_data) = compress_integer_array(data.distribution);
        KV_SERIALIZE(compressed_data)
      }
      else
        KV_SERIALIZE_CONTAINER_POD_AS_BLOB_N(data.distribution, "distribution")
    }
    else
    {
      if (compress)
      {
        KV_SERIALIZE(compressed_data)
        const_cast<std::vector<uint64_t>&>(data.distribution) = decompress_integer_array<uint64_t>(compressed_data);
      }
      else
        KV_SERIALIZE_CONTAINER_POD_AS_BLOB_N(data.distribution, "distribution")
    }
  }
  else
    KV_SERIALIZE_N(data.distribution, "distribution")
  KV_SERIALIZE_N(data.base, "base")
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_OUTPUT_DISTRIBUTION::response)
  KV_SERIALIZE(status)
  KV_SERIALIZE(distributions)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_QUORUM_STATE::request)
  KV_SERIALIZE_OPT(start_height, HEIGHT_SENTINEL_VALUE)
  KV_SERIALIZE_OPT(end_height, HEIGHT_SENTINEL_VALUE)
  KV_SERIALIZE_OPT(quorum_type, ALL_QUORUMS_SENTINEL_VALUE)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_QUORUM_STATE::quorum_t)
  KV_SERIALIZE(validators)
  KV_SERIALIZE(workers)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_QUORUM_STATE::quorum_for_height)
  KV_SERIALIZE(height)
  KV_SERIALIZE(quorum_type)
  KV_SERIALIZE(quorum)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_QUORUM_STATE::response)
  KV_SERIALIZE(status)
  KV_SERIALIZE(quorums)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_NODE_REGISTRATION_CMD_RAW::request)
  KV_SERIALIZE(args)
  KV_SERIALIZE(make_friendly)
  KV_SERIALIZE(staking_requirement)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_NODE_REGISTRATION_CMD_RAW::response)
  KV_SERIALIZE(status)
  KV_SERIALIZE(registration_cmd)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_NODE_REGISTRATION_CMD::contribution_t)
  KV_SERIALIZE(address)
  KV_SERIALIZE(amount)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_NODE_REGISTRATION_CMD::request)
  KV_SERIALIZE(operator_cut)
  KV_SERIALIZE(contributions)
  KV_SERIALIZE(staking_requirement)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_KEYS::response)
  KV_SERIALIZE(service_node_pubkey)
  KV_SERIALIZE(service_node_ed25519_pubkey)
  KV_SERIALIZE(service_node_x25519_pubkey)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_PRIVKEYS::response)
  KV_SERIALIZE(service_node_privkey)
  KV_SERIALIZE(service_node_ed25519_privkey)
  KV_SERIALIZE(service_node_x25519_privkey)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_STAKING_REQUIREMENT::request)
  KV_SERIALIZE(height)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_STAKING_REQUIREMENT::response)
  KV_SERIALIZE(staking_requirement)
  KV_SERIALIZE(height)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_NODE_BLACKLISTED_KEY_IMAGES::entry)
  KV_SERIALIZE(key_image)
  KV_SERIALIZE(unlock_height)
  KV_SERIALIZE(amount)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_SERVICE_NODE_BLACKLISTED_KEY_IMAGES::response)
  KV_SERIALIZE(blacklist)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_CHECKPOINTS::request)
  KV_SERIALIZE_OPT(start_height, HEIGHT_SENTINEL_VALUE)
  KV_SERIALIZE_OPT(end_height, HEIGHT_SENTINEL_VALUE)
  KV_SERIALIZE_OPT(count, NUM_CHECKPOINTS_TO_QUERY_BY_DEFAULT)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_CHECKPOINTS::quorum_signature_serialized)
  KV_SERIALIZE(voter_index);
  KV_SERIALIZE(signature);
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_CHECKPOINTS::checkpoint_serialized)
  KV_SERIALIZE(version);
  KV_SERIALIZE(type);
  KV_SERIALIZE(height);
  KV_SERIALIZE(block_hash);
  KV_SERIALIZE(signatures);
  KV_SERIALIZE(prev_height);
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(GET_CHECKPOINTS::response)
  KV_SERIALIZE(checkpoints)
  KV_SERIALIZE(status)
  KV_SERIALIZE(untrusted)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(ONS_NAMES_TO_OWNERS::request_entry)
  KV_SERIALIZE(name_hash)
  KV_SERIALIZE(types)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(ONS_NAMES_TO_OWNERS::request)
  KV_SERIALIZE(entries)
  KV_SERIALIZE(include_expired)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(ONS_NAMES_TO_OWNERS::response_entry)
  KV_SERIALIZE(entry_index)
  KV_SERIALIZE_ENUM(type)
  KV_SERIALIZE(name_hash)
  KV_SERIALIZE(owner)
  KV_SERIALIZE(backup_owner)
  KV_SERIALIZE(encrypted_value)
  KV_SERIALIZE(update_height)
  KV_SERIALIZE(expiration_height)
  KV_SERIALIZE(txid)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(ONS_NAMES_TO_OWNERS::response)
  KV_SERIALIZE(entries)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()

KV_SERIALIZE_MAP_CODE_BEGIN(ONS_OWNERS_TO_NAMES::request)
  KV_SERIALIZE(entries)
  KV_SERIALIZE(include_expired)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(ONS_OWNERS_TO_NAMES::response_entry)
  KV_SERIALIZE(request_index)
  KV_SERIALIZE_ENUM(type)
  KV_SERIALIZE(name_hash)
  KV_SERIALIZE(owner)
  KV_SERIALIZE(backup_owner)
  KV_SERIALIZE(encrypted_value)
  KV_SERIALIZE(update_height)
  KV_SERIALIZE(expiration_height)
  KV_SERIALIZE(txid)
KV_SERIALIZE_MAP_CODE_END()


KV_SERIALIZE_MAP_CODE_BEGIN(ONS_OWNERS_TO_NAMES::response)
  KV_SERIALIZE(entries)
  KV_SERIALIZE(status)
KV_SERIALIZE_MAP_CODE_END()
}
