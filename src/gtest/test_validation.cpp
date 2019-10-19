#include <gtest/gtest.h>

#include "consensus/validation.h"
#include "main.h"
#include "utiltest.h"

extern ZCJoinSplit* params;

extern bool ReceivedBlockTransactions(const CBlock &block, CValidationState& state, CBlockIndex *pindexNew, const CDiskBlockPos& pos);

void ExpectOptionalAmount(CAmount expected, boost::optional<CAmount> actual) {
    EXPECT_TRUE((bool)actual);
    if (actual) {
        EXPECT_EQ(expected, *actual);
    }
}

// Fake an empty view
class FakeCoinsViewDB : public CCoinsView {
public:
    FakeCoinsViewDB() {}

    bool GetAnchorAt(const uint256 &rt, ZCIncrementalMerkleTree &tree) const {
        return false;
    }

    bool GetNullifier(const uint256 &nf) const {
        return false;
    }

    bool GetCoins(const uint256 &txid, CCoins &coins) const {
        return false;
    }

    bool HaveCoins(const uint256 &txid) const {
        return false;
    }

    uint256 GetBestBlock() const {
        uint256 a;
        return a;
    }

    uint256 GetBestAnchor() const {
        uint256 a;
        return a;
    }

    bool BatchWrite(CCoinsMap &mapCoins,
                    const uint256 &hashBlock,
                    const uint256 &hashAnchor,
                    CAnchorsMap &mapAnchors,
                    CNullifiersMap &mapNullifiers) {
        return false;
    }

    bool GetStats(CCoinsStats &stats) const {
        return false;
    }
};

TEST(Validation, ContextualCheckInputsPassesWithCoinbase) {
    // Create fake coinbase transaction
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    CTransaction tx(mtx);
    ASSERT_TRUE(tx.IsCoinBase());

    // Fake an empty view
    FakeCoinsViewDB fakeDB;
    CCoinsViewCache view(&fakeDB);

    CValidationState state;
    EXPECT_TRUE(ContextualCheckInputs(tx, state, view, false, 0, false, Params(CBaseChainParams::MAIN).GetConsensus()));
}

TEST(Validation, ReceivedBlockTransactions) {
    auto sk = libzcash::SpendingKey::random();

    // Create a fake genesis block
    CBlock block1;
    block1.vtx.push_back(GetValidReceive(*params, sk, 5, true));
    block1.hashMerkleRoot = block1.BuildMerkleTree();
    CBlockIndex fakeIndex1 {block1};

    // Create a fake child block
    CBlock block2;
    block2.hashPrevBlock = block1.GetHash();
    block2.vtx.push_back(GetValidReceive(*params, sk, 10, true));
    block2.hashMerkleRoot = block2.BuildMerkleTree();
    CBlockIndex fakeIndex2 {block2};
    fakeIndex2.pprev = &fakeIndex1;

    CDiskBlockPos pos1;
    CDiskBlockPos pos2;

    // Set initial state of indices
    ASSERT_TRUE(fakeIndex1.RaiseValidity(BLOCK_VALID_TREE));
    ASSERT_TRUE(fakeIndex2.RaiseValidity(BLOCK_VALID_TREE));
    EXPECT_TRUE(fakeIndex1.IsValid(BLOCK_VALID_TREE));
    EXPECT_TRUE(fakeIndex2.IsValid(BLOCK_VALID_TREE));
    EXPECT_FALSE(fakeIndex1.IsValid(BLOCK_VALID_TRANSACTIONS));
    EXPECT_FALSE(fakeIndex2.IsValid(BLOCK_VALID_TRANSACTIONS));

    // Sprout pool values should not be set
    EXPECT_FALSE((bool)fakeIndex1.nSproutValue);
    EXPECT_FALSE((bool)fakeIndex1.nChainSproutValue);
    EXPECT_FALSE((bool)fakeIndex2.nSproutValue);
    EXPECT_FALSE((bool)fakeIndex2.nChainSproutValue);

    // Mark the second block's transactions as received first
    CValidationState state;
    EXPECT_TRUE(ReceivedBlockTransactions(block2, state, &fakeIndex2, pos2));
    EXPECT_FALSE(fakeIndex1.IsValid(BLOCK_VALID_TRANSACTIONS));
    EXPECT_TRUE(fakeIndex2.IsValid(BLOCK_VALID_TRANSACTIONS));

    // Sprout pool value delta should now be set for the second block,
    // but not any chain totals
    EXPECT_FALSE((bool)fakeIndex1.nSproutValue);
    EXPECT_FALSE((bool)fakeIndex1.nChainSproutValue);
    {
        SCOPED_TRACE("ExpectOptionalAmount call");
        ExpectOptionalAmount(20, fakeIndex2.nSproutValue);
    }
    EXPECT_FALSE((bool)fakeIndex2.nChainSproutValue);

    // Now mark the first block's transactions as received
    EXPECT_TRUE(ReceivedBlockTransactions(block1, state, &fakeIndex1, pos1));
    EXPECT_TRUE(fakeIndex1.IsValid(BLOCK_VALID_TRANSACTIONS));
    EXPECT_TRUE(fakeIndex2.IsValid(BLOCK_VALID_TRANSACTIONS));

    // Sprout pool values should now be set for both blocks
    {
        SCOPED_TRACE("ExpectOptionalAmount call");
        ExpectOptionalAmount(10, fakeIndex1.nSproutValue);
    }
    {
        SCOPED_TRACE("ExpectOptionalAmount call");
        ExpectOptionalAmount(10, fakeIndex1.nChainSproutValue);
    }
    {
        SCOPED_TRACE("ExpectOptionalAmount call");
        ExpectOptionalAmount(20, fakeIndex2.nSproutValue);
    }
    {
        SCOPED_TRACE("ExpectOptionalAmount call");
        ExpectOptionalAmount(30, fakeIndex2.nChainSproutValue);
    }
}
