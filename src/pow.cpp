// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "pow.h"

#include "arith_uint256.h"
#include "chain.h"
#include "chainparams.h"
#include "crypto/equihash.h"
#include "main.h"
#include "primitives/block.h"
#include "streams.h"
#include "uint256.h"
#include "util.h"

#include "sodium.h"

#ifdef ENABLE_RUST
#include "librustzcash.h"
#endif // ENABLE_RUST

/**
 * Manually increase difficulty by a multiplier. Note that because of the use of compact bits, this will
 * only be an approx increase, not a 100% precise increase.
 */
unsigned int IncreaseDifficultyBy(unsigned int nBits, int64_t multiplier, const Consensus::Params& params) {
    arith_uint256 target;
    target.SetCompact(nBits);
    target /= multiplier;
    const arith_uint256 pow_limit = UintToArith256(params.powLimit);
    if (target > pow_limit) {
        target = pow_limit;
    }
    return target.GetCompact();
}

unsigned int GetNextWorkRequired(const CBlockIndex* pindexLast, const CBlockHeader *pblock, const Consensus::Params& params)
{

    const CChainParams& chainparams = Params();

    int nHeight = pindexLast->nHeight + 1;

    arith_uint256 proofOfWorkLimit;
    if (!isForkEnabled(nHeight))
        proofOfWorkLimit = UintToArith256(params.prePowLimit);
    else
        proofOfWorkLimit = UintToArith256(params.powLimit);

    unsigned int nProofOfWorkLimit = proofOfWorkLimit.GetCompact();

    // Genesis block
    if (pindexLast == NULL)
        return nProofOfWorkLimit;

    // For upgrade mainnet forks, we'll adjust the difficulty down for the first nPowAveragingWindow blocks
    if (nHeight >= chainparams.EquihashParamsUpdate() &&
        nHeight < chainparams.EquihashParamsUpdate() + params.nPowAveragingWindow) {

        if (pblock && pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 12) {
            // If > 30 mins, allow min difficulty
            LogPrintf("HC Returning level 1 difficulty %i at height %i\n", nProofOfWorkLimit, nHeight);
            return nProofOfWorkLimit;
        } else if (pblock && pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 6) {
            // If > 15 mins, allow low estimate difficulty
            unsigned int difficulty = IncreaseDifficultyBy(nProofOfWorkLimit, 128, params);
            LogPrintf("HC Returning level 2 difficulty %i at height %i\n", nProofOfWorkLimit, nHeight);
            return difficulty;
        } else if (pblock && pblock->GetBlockTime() > pindexLast->GetBlockTime() + params.nPowTargetSpacing * 2) {
            // If > 5 mins, allow high estimate difficulty
            unsigned int difficulty = IncreaseDifficultyBy(nProofOfWorkLimit, 256, params);
            LogPrintf("HC Returning level 3 difficulty %i at height %i\n", nProofOfWorkLimit, nHeight);
            return difficulty;
        } else {
            // If < 5 mins, fall through, and return the normal difficulty.
            LogPrintf("HC Falling through at height %i\n", nHeight);
        }
    }

    // LWMA-1 activated
    if (pindexLast->nHeight > chainparams.LwmaHeight())
        return LwmaCalculateNextWorkRequired(pindexLast, params);

    // right post fork
    if (!isForkBlock(nHeight) && isForkBlock(nHeight - params.nPowAveragingWindow))
        return nProofOfWorkLimit;

    // right at fork
    if (isForkBlock(nHeight) && !isForkBlock(nHeight - params.nPowAveragingWindow))
        return nProofOfWorkLimit;

    // Find the first block in the averaging interval
    const CBlockIndex* pindexFirst = pindexLast;
    arith_uint256 bnTot {0};
    for (int i = 0; pindexFirst && i < params.nPowAveragingWindow; i++) {
        arith_uint256 bnTmp;
        bnTmp.SetCompact(pindexFirst->nBits);
        bnTot += bnTmp;
        pindexFirst = pindexFirst->pprev;
    }

    // Check we have enough blocks
    if (pindexFirst == NULL)
        return nProofOfWorkLimit;

    arith_uint256 bnAvg {bnTot / params.nPowAveragingWindow};

    bool isFork = isForkBlock(pindexLast->nHeight + 1);
    return CalculateNextWorkRequired(bnAvg, pindexLast->GetMedianTimePast(), pindexFirst->GetMedianTimePast(), params, proofOfWorkLimit, isFork);
}

// LWMA-1 for BTC/Zcash clones
// LWMA has the best response*stability.  It rises slowly & drops fast when needed.
// Copyright (c) 2017-2018 The Bitcoin Gold developers
// Copyright (c) 2018-2019 Zawy
// Copyright (c) 2018 iamstenman (Microbitcoin)
// MIT License
// Algorithm by Zawy, a modification of WT-144 by Tom Harding
// For any changes, patches, updates, etc see
// https://github.com/zawy12/difficulty-algorithms/issues/3#issuecomment-442129791
//  FTL should be lowered to about N*T/20.
//  FTL in BTC clones is MAX_FUTURE_BLOCK_TIME in chain.h.
//  FTL in Zcash & Dash clones need to change the 2*60*60 here:
//  if (block.GetBlockTime() > nAdjustedTime + 2 * 60 * 60)
//  which is around line 3700 in main.cpp in ZEC and validation.cpp in Dash
//  If your coin uses network time instead of node local time, lowering FTL < about 125% of the
//  "revert to node time" rule (70 minutes in BCH, ZEC, & BTC) allows 33% Sybil attack,
//  so revert rule must be ~ FTL/2 instead of 70 minutes. See:
// https://github.com/zcash/zcash/issues/4021

unsigned int LwmaCalculateNextWorkRequired(const CBlockIndex* pindexLast, const Consensus::Params& params)
{
    const CChainParams& chainparams = Params();
    // For T=600, 300, 150 (ie 10, 5, 2.5min blocks) use approximately N=60, 90, 120
    const int64_t T = params.nPowTargetSpacing;
    const int64_t N = chainparams.LwmaAveragingWin();

    // Define a k that will be used to get a proper average after weighting the solvetimes.
    const int64_t k = N * (N + 1) * T / 2;

    const int64_t height = pindexLast->nHeight;
    const arith_uint256 powLimit = UintToArith256(params.powLimit);

    arith_uint256 avgTarget, nextTarget;
    int64_t thisTimestamp, previousTimestamp;
    int64_t sumWeightedSolvetimes = 0, j = 0;

    const CBlockIndex* blockPreviousTimestamp = pindexLast->GetAncestor(height - N);
    previousTimestamp = blockPreviousTimestamp->GetBlockTime();

    // Loop through N most recent blocks.
    for (int64_t i = height - N + 1; i <= height; i++) {
        const CBlockIndex* block = pindexLast->GetAncestor(i);

        // Prevent solvetimes from being negative in a safe way. It must be done like this.
        // In particular, do not attempt anything like  if(solvetime < 0) {solvetime=0;}
        // The +1 ensures new coins do not calculate nextTarget = 0.
        thisTimestamp = (block->GetBlockTime() > previousTimestamp) ?
                            block->GetBlockTime() : previousTimestamp + 1;

       // A 6*T limit will prevent large drops in difficulty from long solvetimes.
        int64_t solvetime = std::min(6 * T, thisTimestamp - previousTimestamp);

       // The following is part of "preventing negative solvetimes".
        previousTimestamp = thisTimestamp;

       // Give linearly higher weight to more recent solvetimes.
        j++;
        sumWeightedSolvetimes += solvetime * j;

        arith_uint256 target;
        target.SetCompact(block->nBits);
        avgTarget += target / N / k; // Dividing by k here prevents an overflow below.
    }
   // Desired equation in next line was nextTarget = avgTarget * sumWeightSolvetimes / k
   // but 1/k was moved to line above to prevent overflow in new coins

    nextTarget = avgTarget * sumWeightedSolvetimes;

    if (nextTarget > powLimit) { nextTarget = powLimit; }

    return nextTarget.GetCompact();
}

// (Original)
unsigned int CalculateNextWorkRequired(arith_uint256 bnAvg,
                                       int64_t nLastBlockTime, int64_t nFirstBlockTime,
                                       const Consensus::Params& params, const arith_uint256 bnPowLimit, bool isFork)
{
    // Limit adjustment step
    // Use medians to prevent time-warp attacks
    int64_t nActualTimespan = nLastBlockTime - nFirstBlockTime;
    LogPrint("pow", "  nActualTimespan = %d  before dampening\n", nActualTimespan);
    nActualTimespan = params.AveragingWindowTimespan(isFork) + (nActualTimespan - params.AveragingWindowTimespan(isFork))/4;
    LogPrint("pow", "  nActualTimespan = %d  before bounds\n", nActualTimespan);

    if (nActualTimespan < params.MinActualTimespan(isFork))
        nActualTimespan = params.MinActualTimespan(isFork);
    if (nActualTimespan > params.MaxActualTimespan(isFork))
        nActualTimespan = params.MaxActualTimespan(isFork);

    // Retarget
    arith_uint256 bnNew {bnAvg};
    bnNew /= params.AveragingWindowTimespan(isFork);
    bnNew *= nActualTimespan;

    if (bnNew > bnPowLimit)
        bnNew = bnPowLimit;

    /// debug print
    LogPrint("pow", "GetNextWorkRequired RETARGET\n");
    LogPrint("pow", "params.AveragingWindowTimespan() = %d    nActualTimespan = %d\n", params.AveragingWindowTimespan(), nActualTimespan);
    LogPrint("pow", "Current average: %08x  %s\n", bnAvg.GetCompact(), bnAvg.ToString());
    LogPrint("pow", "After:  %08x  %s\n", bnNew.GetCompact(), bnNew.ToString());

    return bnNew.GetCompact();
}

bool CheckEquihashSolution(const CBlockHeader *pblock, const CChainParams& params)
{
    uint64_t forkHeight = params.EquihashParamsUpdate();
    unsigned int solution_size = pblock->nSolution.size();

    unsigned int n;
    unsigned int k;
    if (solution_size == params.EquihashSolutionWidth(forkHeight)) {
        n = params.EquihashN(forkHeight);
        k = params.EquihashK(forkHeight);
    } else if (forkHeight > 0 && solution_size == params.EquihashSolutionWidth(forkHeight - 1)) {
        n = params.EquihashN(forkHeight - 1);
        k = params.EquihashK(forkHeight - 1);
    } else {
        return error("CheckEquihashsolution(): invalid solution size");
    }

    // Hash state
    crypto_generichash_blake2b_state state;
    EhInitialiseState(n, k, state);

    // I = the block header minus nonce and solution.
    CEquihashInput I{*pblock};
    // I||V
    CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
    ss << I;
    ss << pblock->nNonce;

    // H(I||V||...
    crypto_generichash_blake2b_update(&state, (unsigned char*)&ss[0], ss.size());

    #ifdef ENABLE_RUST
    // Ensure that our Rust interactions are working in production builds. This is
    // temporary and should be removed.
    {
        assert(librustzcash_xor(0x0f0f0f0f0f0f0f0f, 0x1111111111111111) == 0x1e1e1e1e1e1e1e1e);
    }
    #endif // ENABLE_RUST

    bool isValid;
    EhIsValidSolution(n, k, state, pblock->nSolution, isValid);
    if (!isValid)
        return error("CheckEquihashSolution(): invalid solution");

    return true;
}

bool CheckProofOfWork(uint256 hash, unsigned int nBits, const Consensus::Params& params)
{
    bool fNegative;
    bool fOverflow;
    arith_uint256 bnTarget;

    bnTarget.SetCompact(nBits, &fNegative, &fOverflow);

    // Check range
    if (fNegative || bnTarget == 0 || fOverflow || bnTarget > UintToArith256(params.powLimit))
        return error("CheckProofOfWork(): nBits below minimum work");

    // Check proof of work matches claimed amount
    if (UintToArith256(hash) > bnTarget)
        return error("CheckProofOfWork(): hash doesn't match nBits");

    return true;
}

arith_uint256 GetBlockProof(const CBlockIndex& block)
{
    arith_uint256 bnTarget;
    bool fNegative;
    bool fOverflow;
    bnTarget.SetCompact(block.nBits, &fNegative, &fOverflow);
    if (fNegative || fOverflow || bnTarget == 0)
        return 0;
    // We need to compute 2**256 / (bnTarget+1), but we can't represent 2**256
    // as it's too large for a arith_uint256. However, as 2**256 is at least as large
    // as bnTarget+1, it is equal to ((2**256 - bnTarget - 1) / (bnTarget+1)) + 1,
    // or ~bnTarget / (nTarget+1) + 1.
    return (~bnTarget / (bnTarget + 1)) + 1;
}

int64_t GetBlockProofEquivalentTime(const CBlockIndex& to, const CBlockIndex& from, const CBlockIndex& tip, const Consensus::Params& params)
{
    arith_uint256 r;
    int sign = 1;
    if (to.nChainWork > from.nChainWork) {
        r = to.nChainWork - from.nChainWork;
    } else {
        r = from.nChainWork - to.nChainWork;
        sign = -1;
    }
    r = r * arith_uint256(params.nPowTargetSpacing) / GetBlockProof(tip);
    if (r.bits() > 63) {
        return sign * std::numeric_limits<int64_t>::max();
    }
    return sign * r.GetLow64();
}
