#include "gtest/gtest.h"

#include "common/oxen.h"
#include "cryptonote_core/oxen_name_system.h"
#include "oxen_economy.h"

TEST(oxen_name_system, name_tests)
{
  struct name_test
  {
    std::string name;
    bool allowed;
  };

  name_test const lokinet_names[] = {
      {"a.oxen", true},
      {"domain.oxen", true},
      {"xn--tda.oxen", true}, // ü
      {"xn--Mnchen-Ost-9db.oxen", true}, // München-Ost
      {"xn--fwg93vdaef749it128eiajklmnopqrstu7dwaxyz0a1a2a3a643qhok169a.oxen", true}, // ⸘🌻‽💩🤣♠♡♢♣🂡🂢🂣🂤🂥🂦🂧🂨🂩🂪🂫🂬🂭🂮🂱🂲🂳🂴🂵🂶🂷🂸🂹
      {"abcdefghijklmnopqrstuvwxyz123456.oxen", true}, // Max length = 32 if no hyphen (so that it can't look like a raw address)
      {"a-cdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789a.oxen", true}, // Max length = 63 if there is at least one hyphen

      {"abc.domain.oxen", false},
      {"a", false},
      {"a.loko", false},
      {"a domain name.oxen", false},
      {"-.oxen", false},
      {"a_b.oxen", false},
      {" a.oxen", false},
      {"a.oxen ", false},
      {" a.oxen ", false},
      {"localhost.oxen", false},
      {"localhost", false},
      {"oxen.oxen", false},
      {"snode.oxen", false},
      {"abcdefghijklmnopqrstuvwxyz1234567.oxen", false}, // Too long (no hyphen)
      {"a-cdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz0123456789ab.oxen", false}, // Too long with hyphen
      {"xn--fwg93vdaef749it128eiajklmnopqrstu7dwaxyz0a1a2a3a643qhok169ab.oxen", false}, // invalid (punycode and DNS name parts max at 63)
      {"ab--xyz.oxen", false}, // Double-hyphen at chars 3&4 is reserved by DNS (currently only xn-- is used).
  };

  name_test const session_wallet_names[] = {
      {"Hello", true},
      {"1Hello", true},
      {"1Hello1", true},
      {"_Hello1", true},
      {"1Hello_", true},
      {"_Hello_", true},
      {"999", true},
      {"xn--tda", true},
      {"xn--Mnchen-Ost-9db", true},

      {"-", false},
      {"@", false},
      {"'Hello", false},
      {"@Hello", false},
      {"[Hello", false},
      {"]Hello", false},
      {"Hello ", false},
      {" Hello", false},
      {" Hello ", false},

      {"Hello World", false},
      {"Hello\\ World", false},
      {"\"hello\"", false},
      {"hello\"", false},
      {"\"hello", false},
  };

  for (uint16_t type16 = 0; type16 < static_cast<uint16_t>(lns::mapping_type::_count); type16++)
  {
    auto type = static_cast<lns::mapping_type>(type16);
    if (type == lns::mapping_type::wallet) continue; // Not yet supported
    name_test const *names = lns::is_lokinet_type(type) ? lokinet_names : session_wallet_names;
    size_t names_count     = lns::is_lokinet_type(type) ? oxen::char_count(lokinet_names) : oxen::char_count(session_wallet_names);

    for (size_t i = 0; i < names_count; i++)
    {
      name_test const &entry = names[i];
      ASSERT_EQ(lns::validate_lns_name(type, entry.name), entry.allowed) << "Values were {type=" << type << ", name=\"" << entry.name << "\"}";
    }
  }
}

TEST(oxen_name_system, value_encrypt_and_decrypt)
{
  std::string name         = "my lns name";
  lns::mapping_value value = {};
  value.len                = 32;
  memset(&value.buffer[0], 'a', value.len);

  // The type here is not hugely important for decryption except that lokinet (as opposed to
  // session) doesn't fall back to argon2 decryption if decryption fails.
  constexpr auto type = lns::mapping_type::lokinet;

  // Encryption and Decryption success
  {
    auto mval = value;
    ASSERT_TRUE(mval.encrypt(name));
    ASSERT_FALSE(mval == value);
    ASSERT_TRUE(mval.decrypt(name, type));
    ASSERT_TRUE(mval == value);
  }

  // Decryption Fail: Encrypted value was modified
  {
    auto mval = value;
    ASSERT_FALSE(mval.encrypted);
    ASSERT_TRUE(mval.encrypt(name));
    ASSERT_TRUE(mval.encrypted);

    mval.buffer[0] = 'Z';
    ASSERT_FALSE(mval.decrypt(name, type));
    ASSERT_TRUE(mval.encrypted);
  }

  // Decryption Fail: Name was modified
  {
    std::string name_copy = name;
    auto mval = value;
    ASSERT_TRUE(mval.encrypt(name_copy));

    name_copy[0] = 'z';
    ASSERT_FALSE(mval.decrypt(name_copy, type));
  }
}

TEST(oxen_name_system, value_encrypt_and_decrypt_heavy)
{
  std::string name         = "abcdefg";
  lns::mapping_value value = {};
  value.len                = 33;
  memset(&value.buffer[0], 'a', value.len);

  // Encryption and Decryption success for the older argon2-based encryption key
  {
    auto mval = value;
    auto mval_new = value;
    ASSERT_TRUE(mval.encrypt(name, nullptr, true));
    ASSERT_TRUE(mval_new.encrypt(name, nullptr, false));
    ASSERT_EQ(mval.len + 24, mval_new.len); // New value appends a 24-byte nonce
    ASSERT_TRUE(mval.decrypt(name, lns::mapping_type::session));
    ASSERT_TRUE(mval_new.decrypt(name, lns::mapping_type::session));
    ASSERT_TRUE(mval == value);
    ASSERT_TRUE(mval_new == value);
  }
}
