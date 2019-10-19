#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "chainparams.h"
#include "key.h"
#include "miner.h"
#include "util.h"
#ifdef ENABLE_WALLET
#include "wallet/wallet.h"
#endif

#include <boost/optional.hpp>

using ::testing::Return;

#ifdef ENABLE_WALLET
class MockReserveKey : public CReserveKey {
public:
    MockReserveKey() : CReserveKey(nullptr) { }

    MOCK_METHOD1(GetReservedKey, bool(CPubKey &pubkey));
};
#endif

TEST(Miner, GetMinerScriptPubKey) {
    SelectParams(CBaseChainParams::MAIN);

    boost::optional<CScript> scriptPubKey;
#ifdef ENABLE_WALLET
    MockReserveKey reservekey;
    EXPECT_CALL(reservekey, GetReservedKey(::testing::_))
        .WillRepeatedly(Return(false));
#endif

    // No miner address set
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_FALSE((bool) scriptPubKey);

    mapArgs["-mineraddress"] = "notAnAddress";
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_FALSE((bool) scriptPubKey);

    // Partial address
    mapArgs["-mineraddress"] = "AnNhQ4bYHtdPiWfjXE";
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_FALSE((bool) scriptPubKey);

    // Typo in address
    mapArgs["-mineraddress"] = "AnNhQ4BYHtdPiWfjXEr6GN4kfYxwUP8sf2g";
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_FALSE((bool) scriptPubKey);

    // Set up expected scriptPubKey for t1T8yaLVhNqxA5KJcmiqqFN88e8DNp2PBfF
    CKeyID keyID;
    keyID.SetHex("63042e45c6c6b7b6833819e2bbac35b8710e770c");
    CScript expectedScriptPubKey = CScript() << OP_DUP << OP_HASH160 << ToByteVector(keyID) << OP_EQUALVERIFY << OP_CHECKSIG;

    // Valid address
    mapArgs["-mineraddress"] = "AnNhQ4bYHtdPiWfjXEr6GN4kfYxwUP8sf2g";
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_TRUE((bool) scriptPubKey);
    EXPECT_EQ(expectedScriptPubKey, *scriptPubKey);

    // Valid address with leading whitespace
    mapArgs["-mineraddress"] = "  AnNhQ4bYHtdPiWfjXEr6GN4kfYxwUP8sf2g";
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_TRUE((bool) scriptPubKey);
    EXPECT_EQ(expectedScriptPubKey, *scriptPubKey);

    // Valid address with trailing whitespace
    mapArgs["-mineraddress"] = "AnNhQ4bYHtdPiWfjXEr6GN4kfYxwUP8sf2g  ";
#ifdef ENABLE_WALLET
    scriptPubKey = GetMinerScriptPubKey(reservekey);
#else
    scriptPubKey = GetMinerScriptPubKey();
#endif
    EXPECT_TRUE((bool) scriptPubKey);
    EXPECT_EQ(expectedScriptPubKey, *scriptPubKey);
}
