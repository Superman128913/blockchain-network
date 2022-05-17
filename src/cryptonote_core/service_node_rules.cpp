#include "cryptonote_config.h"
#include "cryptonote_basic/hardfork.h"
#include "common/oxen.h"
#include "epee/int-util.h"
#include <oxenc/endian.h>
#include <limits>
#include <vector>
#include <boost/lexical_cast.hpp>
#include <cfenv>

#include "oxen_economy.h"
#include "service_node_rules.h"

using cryptonote::hf;

namespace service_nodes {

// TODO(oxen): Move to oxen_economy, this will also need access to oxen::exp2
uint64_t get_staking_requirement(cryptonote::network_type nettype, uint64_t height)
{
  if (nettype != cryptonote::network_type::MAINNET)
      return oxen::STAKING_REQUIREMENT_TESTNET;

  if (is_hard_fork_at_least(nettype, hf::hf16_pulse, height))
    return oxen::STAKING_REQUIREMENT;

  if (is_hard_fork_at_least(nettype, hf::hf13_enforce_checkpoints, height))
  {
    constexpr int64_t heights[] = {
        385824,
        429024,
        472224,
        515424,
        558624,
        601824,
        645024,
    };

    constexpr int64_t lsr[] = {
        20458'380815527,
        19332'319724305,
        18438'564443912,
        17729'190407764,
        17166'159862153,
        16719'282221956,
        16364'595203882,
    };

    assert(static_cast<int64_t>(height) >= heights[0]);
    constexpr uint64_t LAST_HEIGHT      = heights[oxen::array_count(heights) - 1];
    constexpr uint64_t LAST_REQUIREMENT = lsr    [oxen::array_count(lsr) - 1];
    if (height >= LAST_HEIGHT)
        return LAST_REQUIREMENT;

    size_t i = 0;
    for (size_t index = 1; index < oxen::array_count(heights); index++)
    {
      if (heights[index] > static_cast<int64_t>(height))
      {
        i = (index - 1);
        break;
      }
    }

    int64_t H      = height;
    int64_t result = lsr[i] + (H - heights[i]) * ((lsr[i + 1] - lsr[i]) / (heights[i + 1] - heights[i]));
    return static_cast<uint64_t>(result);
  }

  uint64_t hardfork_height = 101250;
  if (height < hardfork_height) height = hardfork_height;

  uint64_t height_adjusted = height - hardfork_height;
  uint64_t base = 0, variable = 0;
  std::fesetround(FE_TONEAREST);
  if (is_hard_fork_at_least(nettype, hf::hf11_infinite_staking, height))
  {
    base     = 15000 * oxen::COIN;
    variable = (25007.0 * oxen::COIN) / oxen::exp2(height_adjusted/129600.0);
  }
  else
  {
    base      = 10000 * oxen::COIN;
    variable  = (35000.0 * oxen::COIN) / oxen::exp2(height_adjusted/129600.0);
  }

  uint64_t result = base + variable;
  return result;
}

uint64_t portions_to_amount(uint64_t portions, uint64_t staking_requirement)
{
  uint64_t hi, lo, resulthi, resultlo;
  lo = mul128(staking_requirement, portions, &hi);
  div128_64(hi, lo, cryptonote::old::STAKING_PORTIONS, &resulthi, &resultlo);
  return resultlo;
}

bool check_service_node_portions(hf hf_version, const std::vector<uint64_t>& portions)
{
  uint64_t portion_fuzz = hf_version >= hf::hf19 ? PORTION_FUZZ : 0;

  const size_t max_contributors = hf_version >= hf::hf19 ? oxen::MAX_CONTRIBUTORS_HF19 : oxen::MAX_CONTRIBUTORS_V1;
  if (portions.size() > max_contributors) {
    LOG_PRINT_L1("Registration tx rejected: too many contributors (" << portions.size() << " > " << max_contributors << ")");
    return false;
  }

  if (portions[0] < MINIMUM_OPERATOR_PORTION - portion_fuzz)
  {
    LOG_PRINT_L1("Register TX rejected: TX does not have sufficient operator stake (" << portions[0] << " < " << MINIMUM_OPERATOR_PORTION << ")");
    return false;
  }

  uint64_t reserved = 0;
  for (auto i = 0u; i < portions.size(); ++i)
  {

    uint64_t min_portions = get_min_node_contribution(hf_version, cryptonote::old::STAKING_PORTIONS, reserved, i);

    if (min_portions > portion_fuzz)
      min_portions -= portion_fuzz;

    if (portions[i] < min_portions) {
      LOG_PRINT_L1("Registration tx rejected: portion " << i << " too small (" << portions[i] << " < " << min_portions << ")");
      return false;
    }
    reserved += portions[i];
  }

  if (reserved > cryptonote::old::STAKING_PORTIONS) {
    LOG_PRINT_L1("Registration tx rejected: total reserved amount too large");
    return false;
  }
  return true;
}

crypto::hash generate_request_stake_unlock_hash(uint32_t nonce)
{
  static_assert(sizeof(crypto::hash) == 8 * sizeof(uint32_t) && alignof(crypto::hash) >= alignof(uint32_t));
  crypto::hash result;
  oxenc::host_to_little_inplace(nonce);
  for (size_t i = 0; i < 8; i++)
    reinterpret_cast<uint32_t*>(result.data)[i] = nonce;
  return result;
}

uint64_t get_locked_key_image_unlock_height(cryptonote::network_type nettype, uint64_t node_register_height, uint64_t curr_height)
{
  uint64_t blocks_to_lock = staking_num_lock_blocks(nettype);
  uint64_t result         = curr_height + (blocks_to_lock / 2);
  return result;
}

static uint64_t get_min_node_contribution_pre_v11(uint64_t staking_requirement, uint64_t total_reserved)
{
  return std::min(staking_requirement - total_reserved, staking_requirement / oxen::MAX_CONTRIBUTORS_V1);
}

uint64_t get_max_node_contribution(hf version, uint64_t staking_requirement, uint64_t total_reserved)
{
  if (version >= hf::hf16_pulse)
    return (staking_requirement - total_reserved) * cryptonote::MAXIMUM_ACCEPTABLE_STAKE::num
      / cryptonote::MAXIMUM_ACCEPTABLE_STAKE::den;
  return std::numeric_limits<uint64_t>::max();
}

uint64_t get_min_node_contribution(hf version, uint64_t staking_requirement, uint64_t total_reserved, size_t num_contributions)
{
  if (version < hf::hf11_infinite_staking)
    return get_min_node_contribution_pre_v11(staking_requirement, total_reserved);

  const uint64_t needed = staking_requirement - total_reserved;

  const size_t max_contributors = version >= hf::hf19 ? oxen::MAX_CONTRIBUTORS_HF19 : oxen::MAX_CONTRIBUTORS_V1;
  assert(max_contributors > num_contributions);
  if (max_contributors <= num_contributions) return UINT64_MAX;

  const size_t num_contributions_remaining_avail = max_contributors - num_contributions;
  return needed / num_contributions_remaining_avail;
}

uint64_t get_min_node_contribution_in_portions(hf version, uint64_t staking_requirement, uint64_t total_reserved, size_t num_contributions)
{
  uint64_t atomic_amount = get_min_node_contribution(version, staking_requirement, total_reserved, num_contributions);
  uint64_t result        = (atomic_amount == UINT64_MAX) ? UINT64_MAX : (get_portions_to_make_amount(staking_requirement, atomic_amount));
  return result;
}

uint64_t get_portions_to_make_amount(uint64_t staking_requirement, uint64_t amount, uint64_t max_portions)
{
  uint64_t lo, hi, resulthi, resultlo;
  lo = mul128(amount, max_portions, &hi);
  if (lo > UINT64_MAX - (staking_requirement - 1))
    hi++;
  lo += staking_requirement-1;
  div128_64(hi, lo, staking_requirement, &resulthi, &resultlo);
  return resultlo;
}

static bool get_portions_from_percent(double cur_percent, uint64_t& portions) {
  if(cur_percent < 0.0 || cur_percent > 100.0) return false;

  // Fix for truncation issue when operator cut = 100 for a pool Service Node.
  if (cur_percent == 100.0)
  {
    portions = cryptonote::old::STAKING_PORTIONS;
  }
  else
  {
    portions = (cur_percent / 100.0) * (double)cryptonote::old::STAKING_PORTIONS;
  }

  return true;
}

bool get_portions_from_percent_str(std::string cut_str, uint64_t& portions) {

  if(!cut_str.empty() && cut_str.back() == '%')
  {
    cut_str.pop_back();
  }

  double cut_percent;
  try
  {
    cut_percent = boost::lexical_cast<double>(cut_str);
  }
  catch(...)
  {
    return false;
  }

  return get_portions_from_percent(cut_percent, portions);
}

} // namespace service_nodes
