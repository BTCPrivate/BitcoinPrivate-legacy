#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "consensus/validation.h"
#include "main.h"
#include "zcash/Proof.hpp"
#include "base58.h"

class MockCValidationState : public CValidationState {
public:
    MOCK_METHOD5(DoS, bool(int level, bool ret,
             unsigned char chRejectCodeIn, std::string strRejectReasonIn,
             bool corruptionIn));
    MOCK_METHOD3(Invalid, bool(bool ret,
                 unsigned char _chRejectCode, std::string _strRejectReason));
    MOCK_METHOD1(Error, bool(std::string strRejectReasonIn));
    MOCK_CONST_METHOD0(IsValid, bool());
    MOCK_CONST_METHOD0(IsInvalid, bool());
    MOCK_CONST_METHOD0(IsError, bool());
    MOCK_CONST_METHOD1(IsInvalid, bool(int &nDoSOut));
    MOCK_CONST_METHOD0(CorruptionPossible, bool());
    MOCK_CONST_METHOD0(GetRejectCode, unsigned char());
    MOCK_CONST_METHOD0(GetRejectReason, std::string());
};

TEST(CheckBlock, VersionTooLow) {
    auto verifier = libzcash::ProofVerifier::Strict();

    CBlock block;
    block.nVersion = 1;

    MockCValidationState state;
    EXPECT_CALL(state, DoS(100, false, REJECT_INVALID, "version-too-low", false)).Times(1);
    EXPECT_FALSE(CheckBlock(block, state, verifier, false, false));
}


// Test that a tx with negative version is still rejected
// by CheckBlock under consensus rules.
TEST(CheckBlock, BlockRejectsBadVersion) {
    SelectParams(CBaseChainParams::MAIN);

    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout.SetNull();
    mtx.vin[0].scriptSig = CScript() << 1 << OP_0;
    mtx.vout.resize(1);
    mtx.vout[0].scriptPubKey = CScript() << OP_TRUE;
    mtx.vout[0].nValue = 0;

    mtx.nVersion = -1;

    CTransaction tx {mtx};
    CBlock block;
    // explicitly set to minimum, otherwise a preliminary check will fail
    block.nVersion = MIN_BLOCK_VERSION;
    block.vtx.push_back(tx);

    MockCValidationState state;

    auto verifier = libzcash::ProofVerifier::Strict();

    EXPECT_CALL(state, DoS(100, false, REJECT_INVALID, "bad-txns-version-too-low", false)).Times(1);
    EXPECT_FALSE(CheckBlock(block, state, verifier, false, false));
}


class ContextualCheckBlockTest : public ::testing::Test {
protected:
    virtual void SetUp() {
        SelectParams(CBaseChainParams::MAIN);
    }

    // Returns a valid but empty mutable transaction at given block height.
    CMutableTransaction GetBlockTxWithHeight(const int height) {
        CMutableTransaction mtx;

        // No inputs.
        mtx.vin.resize(1);
        mtx.vin[0].prevout.SetNull();

        // Set height
        mtx.vin[0].scriptSig = CScript() << height << OP_0;

        mtx.vout.resize(1);
        mtx.vout[0].scriptPubKey = CScript() << OP_TRUE;
        mtx.vout[0].nValue = 0;

        // CAmount reward = GetBlockSubsidy(height, Params().GetConsensus());

		// for (Fork::CommunityFundType cfType=Fork::CommunityFundType::FOUNDATION; cfType < Fork::CommunityFundType::ENDTYPE; cfType = Fork::CommunityFundType(cfType + 1)) {
		// 	CAmount vCommunityFund = ForkManager::getInstance().getCommunityFundReward(height, reward, cfType);
		// 	if (vCommunityFund > 0) {
				// Take some reward away from miners
				mtx.vout[0].nValue = 50 * COIN;
				// And give it to the community
				// mtx.vout.push_back(CTxOut(vCommunityFund, Params().GetCommunityFundScriptAtHeight(height, cfType)));
			// }
		// }
        return mtx;
    }

    // Expects a height-n block containing a given transaction to pass
    // ContextualCheckBlock. You should not call it without
    // calling a SCOPED_TRACE macro first to usefully label any failures.
    void ExpectValidBlockFromTx(const CTransaction& tx, const int height) {
        // Create a block and add the transaction to it.
        CBlock block;
        block.vtx.push_back(tx);

        // Set the previous block index with the passed heigth
        CBlock prev;
        CBlockIndex indexPrev {prev};
        indexPrev.nHeight = height - 1;

        // We now expect this to be a valid block.
        MockCValidationState state;
        EXPECT_TRUE(ContextualCheckBlock(block, state, &indexPrev));
    }

    // Expects a height-1 block containing a given transaction to fail
    // ContextualCheckBlock. You should not call it without
    // calling a SCOPED_TRACE macro first to usefully label any failures.
    void ExpectInvalidBlockFromTx(
        const CTransaction& tx, const int height, int level, std::string reason)
    {
        // Create a block and add the transaction to it.
        CBlock block;
        block.vtx.push_back(tx);

        // Set the previous block index with the passed heigth
        CBlock prev;
        CBlockIndex indexPrev {prev};
        indexPrev.nHeight = height - 1;

        // We now expect this to be an invalid block, for the given reason.
        MockCValidationState state;
        EXPECT_CALL(state, DoS(level, false, REJECT_INVALID, reason, false)).Times(1);
        EXPECT_FALSE(ContextualCheckBlock(block, state, &indexPrev));
    }



};



TEST_F(ContextualCheckBlockTest, BadCoinbaseHeight) {
    // Put a transaction in a block with no height in scriptSig
    CMutableTransaction mtx;
    mtx.vin.resize(1);
    mtx.vin[0].prevout.SetNull();
    mtx.vin[0].scriptSig = CScript() << OP_0; // missing height
    mtx.vout.resize(1);
    mtx.vout[0].scriptPubKey = CScript() << OP_TRUE;
    mtx.vout[0].nValue = 0;

    CBlock block;
    block.vtx.push_back(mtx);

    // Treating block as genesis (no prev blocks) should pass
    MockCValidationState state;
    EXPECT_TRUE(ContextualCheckBlock(block, state, NULL));

    // Treating block as non-genesis should fail
    //mtx.vout.push_back(CTxOut(GetBlockSubsidy(1, Params().GetConsensus())/5, Params().GetFoundersRewardScriptAtHeight(1))); // disabled for Bitcoin Private
    CTransaction tx2 {mtx};
    block.vtx[0] = tx2;
    CBlock prev;
    CBlockIndex indexPrev {prev};
    indexPrev.nHeight = 0;
    EXPECT_CALL(state, DoS(100, false, REJECT_INVALID, "bad-cb-height", false)).Times(1);
    EXPECT_FALSE(ContextualCheckBlock(block, state, &indexPrev));

    // Setting to an incorrect height should fail
    mtx.vin[0].scriptSig = CScript() << 2 << OP_0;
    CTransaction tx3 {mtx};
    block.vtx[0] = tx3;
    EXPECT_CALL(state, DoS(100, false, REJECT_INVALID, "bad-cb-height", false)).Times(1);
    EXPECT_FALSE(ContextualCheckBlock(block, state, &indexPrev));

    // After correcting the scriptSig, should pass
    mtx.vin[0].scriptSig = CScript() << 1 << OP_0;
    CTransaction tx4 {mtx};
    block.vtx[0] = tx4;
    EXPECT_TRUE(ContextualCheckBlock(block, state, &indexPrev));
}

// TODO check this: is it meaningful? Why is it called Sprout?
TEST_F(ContextualCheckBlockTest, BlockSproutRulesAcceptSproutTx) {
    CMutableTransaction mtx = GetBlockTxWithHeight(1);

    // Make it a Sprout transaction w/o JoinSplits
    mtx.nVersion = 1;

    SCOPED_TRACE("BlockShieldRulesAcceptSproutTx");
    ExpectValidBlockFromTx(CTransaction(mtx), 0);
}


class ContextualTxsCheckBlockTest : public ContextualCheckBlockTest {
protected:
	void TestTxsAcceptanceRules(CBaseChainParams::Network network, int expectedGrothTxSupportHeight) {
	        SelectParams(network);

	    	CMutableTransaction mtx = GetBlockTxWithHeight(expectedGrothTxSupportHeight - 1);

	        // Make it a Groth transaction: since nHeight is below 200, it will fail
	        mtx.nVersion = GROTH_TX_VERSION;
	        {
	            SCOPED_TRACE("BlockShieldRulesRejectGrothTx");
	            ExpectInvalidBlockFromTx(CTransaction(mtx), expectedGrothTxSupportHeight - 1, 0, "bad-tx-shielded-version-too-low");
	        }

	        // Make it a PHGR transaction: since nHeight is below 200, it will succeed
	        mtx.nVersion = PHGR_TX_VERSION;
	        {
	            SCOPED_TRACE("BlockShieldRulesAcceptPhgrTx");
	            ExpectValidBlockFromTx(CTransaction(mtx), expectedGrothTxSupportHeight - 1);
	        }

	        mtx = GetBlockTxWithHeight(expectedGrothTxSupportHeight);
	        mtx.nVersion = GROTH_TX_VERSION;
	        {
	            SCOPED_TRACE("BlockShieldRulesAcceptGrothTx");
	            ExpectValidBlockFromTx(CTransaction(mtx), expectedGrothTxSupportHeight);
	        }

	        mtx.nVersion = PHGR_TX_VERSION;
	        {
	            SCOPED_TRACE("BlockShieldRulesRejectPhgrTx");
	            ExpectInvalidBlockFromTx(CTransaction(mtx), expectedGrothTxSupportHeight, 100, "bad-tx-shielded-version-too-low");
	        }

	    }
};


TEST_F(ContextualTxsCheckBlockTest, BlockShieldRulesRejectOtherTx) {

	TestTxsAcceptanceRules(CBaseChainParams::REGTEST, 200);
	TestTxsAcceptanceRules(CBaseChainParams::TESTNET, 3000);
	TestTxsAcceptanceRules(CBaseChainParams::MAIN, 50000);

}
