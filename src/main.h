// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_MAIN_H
#define BITCOIN_MAIN_H

#if defined(HAVE_CONFIG_H)
#include "config/bitcoin-config.h"
#endif

#include "amount.h"
#include "chain.h"
#include "chainparams.h"
#include "coins.h"
#include "consensus/consensus.h"
#include "net.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "spentindex.h"
#include "script/script.h"
#include "script/sigcache.h"
#include "script/standard.h"
#include "sync.h"
#include "tinyformat.h"
#include "txmempool.h"
#include "uint256.h"
#include "versionbits.h"

#include <algorithm>
#include <exception>
#include <map>
#include <set>
#include <stdint.h>
#include <string>
#include <utility>
#include <vector>

#include <boost/unordered_map.hpp>

class CBlockIndex;
class CBlockTreeDB;
class CBloomFilter;
class CInv;
class CScriptCheck;
class CValidationInterface;
class CValidationState;

struct CNodeStateStats;

/** Default for -blockmaxsize and -blockminsize, which control the range of sizes the mining code will create **/
static const unsigned int DEFAULT_BLOCK_MAX_SIZE = MAX_BLOCK_SIZE;
static const unsigned int DEFAULT_BLOCK_MIN_SIZE = 0;
/** Default for -blockprioritysize, maximum space for zero/low-fee transactions **/
static const unsigned int DEFAULT_BLOCK_PRIORITY_SIZE = DEFAULT_BLOCK_MAX_SIZE / 2;
/** Default for accepting alerts from the P2P network. */
static const bool DEFAULT_ALERTS = true;
/** Minimum alert priority for enabling safe mode. */
static const int ALERT_PRIORITY_SAFE_MODE = 4000;
/** Maximum number of signature check operations in an IsStandard() P2SH script */
static const unsigned int MAX_P2SH_SIGOPS = 15;
/** The maximum number of sigops we're willing to relay/mine in a single tx */
static const unsigned int MAX_STANDARD_TX_SIGOPS = MAX_BLOCK_SIGOPS/5;
/** Default for -minrelaytxfee, minimum relay fee for transactions */
static const unsigned int DEFAULT_MIN_RELAY_TX_FEE = 100;
/** Default for -maxorphantx, maximum number of orphan transactions kept in memory */
static const unsigned int DEFAULT_MAX_ORPHAN_TRANSACTIONS = 100;
/** The maximum size of a blk?????.dat file (since 0.8) */
static const unsigned int MAX_BLOCKFILE_SIZE = 0x8000000; // 128 MiB
/** The pre-allocation chunk size for blk?????.dat files (since 0.8) */
static const unsigned int BLOCKFILE_CHUNK_SIZE = 0x1000000; // 16 MiB
/** The pre-allocation chunk size for rev?????.dat files (since 0.8) */
static const unsigned int UNDOFILE_CHUNK_SIZE = 0x100000; // 1 MiB
/** Maximum number of script-checking threads allowed */
static const int MAX_SCRIPTCHECK_THREADS = 16;
/** -par default (number of script-checking threads, 0 = auto) */
static const int DEFAULT_SCRIPTCHECK_THREADS = 0;
/** Number of blocks that can be requested at any given time from a single peer. */
static const int MAX_BLOCKS_IN_TRANSIT_PER_PEER = 16;
/** Timeout in seconds during which a peer must stall block download progress before being disconnected. */
static const unsigned int BLOCK_STALLING_TIMEOUT = 2;
/** Number of headers sent in one getheaders result. We rely on the assumption that if a peer sends
 *  less than this number, we reached its tip. Changing this value is a protocol upgrade. */
static const unsigned int MAX_HEADERS_RESULTS = 160;
/** Size of the "block download window": how far ahead of our current height do we fetch?
 *  Larger windows tolerate larger download speed differences between peer, but increase the potential
 *  degree of disordering of blocks on disk (which make reindexing and in the future perhaps pruning
 *  harder). We'll probably want to make this a per-peer adaptive value at some point. */
static const unsigned int BLOCK_DOWNLOAD_WINDOW = 1024;
/** Time to wait (in seconds) between writing blocks/block index to disk. */
static const unsigned int DATABASE_WRITE_INTERVAL = 60 * 60;
/** Time to wait (in seconds) between flushing chainstate to disk. */
static const unsigned int DATABASE_FLUSH_INTERVAL = 24 * 60 * 60;
/** Maximum length of reject messages. */
static const unsigned int MAX_REJECT_MESSAGE_LENGTH = 111;

static const bool DEFAULT_ADDRESSINDEX = false;
static const bool DEFAULT_TIMESTAMPINDEX = false;
static const bool DEFAULT_SPENTINDEX = false;
static const unsigned int DEFAULT_DB_MAX_OPEN_FILES = 1000;
static const bool DEFAULT_DB_COMPRESSION = true;

// Sanity check the magic numbers when we change them
BOOST_STATIC_ASSERT(DEFAULT_BLOCK_MAX_SIZE <= MAX_BLOCK_SIZE);
BOOST_STATIC_ASSERT(DEFAULT_BLOCK_PRIORITY_SIZE <= DEFAULT_BLOCK_MAX_SIZE);

#define equihash_parameters_acceptable(N, K) \
    ((CBlockHeader::HEADER_SIZE + equihash_solution_size(N, K))*MAX_HEADERS_RESULTS < \
     MAX_PROTOCOL_MESSAGE_LENGTH-1000)

struct BlockHasher
{
    size_t operator()(const uint256& hash) const { return hash.GetCheapHash(); }
};

extern CScript COINBASE_FLAGS;
extern CCriticalSection cs_main;
extern CTxMemPool mempool;
typedef boost::unordered_map<uint256, CBlockIndex*, BlockHasher> BlockMap;
extern BlockMap mapBlockIndex;
extern uint64_t nLastBlockTx;
extern uint64_t nLastBlockSize;
extern const std::string strMessageMagic;
extern CWaitableCriticalSection csBestBlock;
extern CConditionVariable cvBlockChange;
extern bool fExperimentalMode;
extern bool fImporting;
extern bool fReindex;
extern int nScriptCheckThreads;
extern bool fTxIndex;
extern bool fIsBareMultisigStd;
extern bool fCheckBlockIndex;
extern bool fCheckpointsEnabled;
// TODO: remove this flag by structuring our code such that
// it is unneeded for testing
extern bool fCoinbaseEnforcedProtectionEnabled;
extern size_t nCoinCacheUsage;
extern CFeeRate minRelayTxFee;
extern bool fAlerts;

/** Best header we've seen so far (used for getheaders queries' starting points). */
extern CBlockIndex *pindexBestHeader;

/** Minimum disk space required - used in CheckDiskSpace() */
static const uint64_t nMinDiskSpace = 52428800;

/** Pruning-related variables and constants */
/** True if any block files have ever been pruned. */
extern bool fHavePruned;
/** True if we're running in -prune mode. */
extern bool fPruneMode;
/** Number of MiB of block files that we're trying to stay below. */
extern uint64_t nPruneTarget;
/** Block files containing a block-height within MIN_BLOCKS_TO_KEEP of chainActive.Tip() will not be pruned. */
static const unsigned int MIN_BLOCKS_TO_KEEP = 288;

// Require that user allocate at least 550MB for block & undo files (blk???.dat and rev???.dat)
// At 1MB per block, 288 blocks = 288MB.
// Add 15% for Undo data = 331MB
// Add 20% for Orphan block rate = 397MB
// We want the low water mark after pruning to be at least 397 MB and since we prune in
// full block file chunks, we need the high water mark which triggers the prune to be
// one 128MB block file + added 15% undo data = 147MB greater for a total of 545MB
// Setting the target to > than 550MB will make it likely we can respect the target.
static const uint64_t MIN_DISK_SPACE_FOR_BLOCK_FILES = 550 * 1024 * 1024;

/** Register with a network node to receive its signals */
void RegisterNodeSignals(CNodeSignals& nodeSignals);
/** Unregister a network node */
void UnregisterNodeSignals(CNodeSignals& nodeSignals);

/**
 * Process an incoming block. This only returns after the best known valid
 * block is made active. Note that it does not, however, guarantee that the
 * specific block passed to it has been checked for validity!
 *
 * @param[out]  state   This may be set to an Error state if any error occurred processing it, including during validation/connection/etc of otherwise unrelated blocks during reorganisation; or it may be set to an Invalid state if pblock is itself invalid (but this is not guaranteed even when the block is checked). If you want to *possibly* get feedback on whether pblock is valid, you must also install a CValidationInterface (see validationinterface.h) - this will have its BlockChecked method called whenever *any* block completes validation.
 * @param[in]   pfrom   The node which we are receiving the block from; it is added to mapBlockSource and may be penalised if the block is invalid.
 * @param[in]   pblock  The block we want to process.
 * @param[in]   fForceProcessing Process this block even if unrequested; used for non-network block sources and whitelisted peers.
 * @param[out]  dbp     If pblock is stored to disk (or already there), this will be set to its location.
 * @return True if state.IsValid()
 */
bool ProcessNewBlock(CValidationState &state, CNode* pfrom, CBlock* pblock, bool fForceProcessing, CDiskBlockPos *dbp
#ifdef FORK_CB_INPUT
                , bool fCalledFromMiner = false
#endif
                    );

/** Check whether enough disk space is available for an incoming block */
bool CheckDiskSpace(uint64_t nAdditionalBytes = 0);
/** Open a block file (blk?????.dat) */
FILE* OpenBlockFile(const CDiskBlockPos &pos, bool fReadOnly = false);
/** Open an undo file (rev?????.dat) */
FILE* OpenUndoFile(const CDiskBlockPos &pos, bool fReadOnly = false);
/** Translation to a filesystem path */
boost::filesystem::path GetBlockPosFilename(const CDiskBlockPos &pos, const char *prefix);
/** Import blocks from an external file */
bool LoadExternalBlockFile(FILE* fileIn, CDiskBlockPos *dbp = NULL);
/** Initialize a new block tree database + block data on disk */
bool InitBlockIndex();
/** Load the block tree and coins database from disk */
bool LoadBlockIndex();
/** Unload database information */
void UnloadBlockIndex();
/** Process protocol messages received from a given node */
bool ProcessMessages(CNode* pfrom);
/**
 * Send queued protocol messages to be sent to a give node.
 *
 * @param[in]   pto             The node which we are sending messages to.
 * @param[in]   fSendTrickle    When true send the trickled data, otherwise trickle the data until true.
 */
bool SendMessages(CNode* pto, bool fSendTrickle);
/** Run an instance of the script checking thread */
void ThreadScriptCheck();
/** Try to detect Partition (network isolation) attacks against us */
void PartitionCheck(bool (*initialDownloadCheck)(), CCriticalSection& cs, const CBlockIndex *const &bestHeader, int64_t nPowTargetSpacing);
/** Check whether we are doing an initial block download (synchronizing from disk or network) */
bool IsInitialBlockDownload(bool includeFork=false);
/** Format a string that describes several potential problems detected by the core */
std::string GetWarnings(const std::string& strFor);
/** Retrieve a transaction (from memory pool, or from disk, if possible) */
bool GetTransaction(const uint256 &hash, CTransaction &tx, uint256 &hashBlock, bool fAllowSlow = false);
/** Find the best known block, and make it the tip of the block chain */
bool ActivateBestChain(CValidationState &state, CBlock *pblock = NULL);
CAmount GetBlockSubsidy(int nHeight, const Consensus::Params& consensusParams);

/**
 * Prune block and undo files (blk???.dat and undo???.dat) so that the disk space used is less than a user-defined target.
 * The user sets the target (in MB) on the command line or in config file.  This will be run on startup and whenever new
 * space is allocated in a block or undo file, staying below the target. Changing back to unpruned requires a reindex
 * (which in this case means the blockchain must be re-downloaded.)
 *
 * Pruning functions are called from FlushStateToDisk when the global fCheckForPruning flag has been set.
 * Block and undo files are deleted in lock-step (when blk00003.dat is deleted, so is rev00003.dat.)
 * Pruning cannot take place until the longest chain is at least a certain length (100000 on mainnet, 1000 on testnet, 10 on regtest).
 * Pruning will never delete a block within a defined distance (currently 288) from the active chain's tip.
 * The block index is updated by unsetting HAVE_DATA and HAVE_UNDO for any blocks that were stored in the deleted files.
 * A db flag records the fact that at least some block files have been pruned.
 *
 * @param[out]   setFilesToPrune   The set of file indices that can be unlinked will be returned
 */
void FindFilesToPrune(std::set<int>& setFilesToPrune);

/**
 *  Actually unlink the specified files
 */
void UnlinkPrunedFiles(std::set<int>& setFilesToPrune);

/** Create a new block index entry for a given block hash */
CBlockIndex * InsertBlockIndex(uint256 hash);
/** Get statistics from node state */
bool GetNodeStateStats(NodeId nodeid, CNodeStateStats &stats);
/** Increase a node's misbehavior score. */
void Misbehaving(NodeId nodeid, int howmuch);
/** Flush all state, indexes and buffers to disk. */
void FlushStateToDisk();
/** Prune block files and flush state to disk. */
void PruneAndFlush();

/** (try to) add transaction to memory pool **/
bool AcceptToMemoryPool(CTxMemPool& pool, CValidationState &state, const CTransaction &tx, bool fLimitFree,
                        bool* pfMissingInputs, bool fRejectAbsurdFee=false);


/** Get the BIP9 state for a given deployment at the current tip. */
ThresholdState VersionBitsTipState(const Consensus::Params& params, Consensus::DeploymentPos pos);

struct CNodeStateStats {
    int nMisbehavior;
    int nSyncHeight;
    int nCommonHeight;
    std::vector<int> vHeightInFlight;
};

struct CTimestampIndexIteratorKey {
     unsigned int timestamp;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 4;
     }
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata32be(s, timestamp);
     }
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         timestamp = ser_readdata32be(s);
     }
 
     CTimestampIndexIteratorKey(unsigned int time) {
         timestamp = time;
     }
 
     CTimestampIndexIteratorKey() {
         SetNull();
     }
 
     void SetNull() {
         timestamp = 0;
     }
 };
 
 struct CTimestampIndexKey {
     unsigned int timestamp;
     uint256 blockHash;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 36;
     }
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata32be(s, timestamp);
         blockHash.Serialize(s, nType, nVersion);
     }
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         timestamp = ser_readdata32be(s);
         blockHash.Unserialize(s, nType, nVersion);
     }
 
     CTimestampIndexKey(unsigned int time, uint256 hash) {
         timestamp = time;
         blockHash = hash;
     }
 
     CTimestampIndexKey() {
         SetNull();
     }
 
     void SetNull() {
         timestamp = 0;
         blockHash.SetNull();
     }
 };
 
 struct CTimestampBlockIndexKey {
     uint256 blockHash;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 32;
     }
 
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         blockHash.Serialize(s, nType, nVersion);
     }
 
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         blockHash.Unserialize(s, nType, nVersion);
     }
 
     CTimestampBlockIndexKey(uint256 hash) {
         blockHash = hash;
     }
 
     CTimestampBlockIndexKey() {
         SetNull();
     }
 
     void SetNull() {
         blockHash.SetNull();
     }
 };
 
 struct CTimestampBlockIndexValue {
     unsigned int ltimestamp;
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 4;
     }
 
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata32be(s, ltimestamp);
     }
 
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         ltimestamp = ser_readdata32be(s);
     }
 
     CTimestampBlockIndexValue (unsigned int time) {
         ltimestamp = time;
     }
 
     CTimestampBlockIndexValue() {
         SetNull();
     }
 
     void SetNull() {
         ltimestamp = 0;
     }
 };
 
 struct CAddressUnspentKey {
     unsigned int type;
     uint160 hashBytes;
     uint256 txhash;
     size_t index;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 57;
     }
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata8(s, type);
         hashBytes.Serialize(s, nType, nVersion);
         txhash.Serialize(s, nType, nVersion);
         ser_writedata32(s, index);
     }
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         type = ser_readdata8(s);
         hashBytes.Unserialize(s, nType, nVersion);
         txhash.Unserialize(s, nType, nVersion);
         index = ser_readdata32(s);
     }
 
     CAddressUnspentKey(unsigned int addressType, uint160 addressHash, uint256 txid, size_t indexValue) {
         type = addressType;
         hashBytes = addressHash;
         txhash = txid;
         index = indexValue;
     }
 
     CAddressUnspentKey() {
         SetNull();
     }
 
     void SetNull() {
         type = 0;
         hashBytes.SetNull();
         txhash.SetNull();
         index = 0;
     }
 };
 
 struct CAddressUnspentValue {
     CAmount satoshis;
     CScript script;
     int blockHeight;
 
     ADD_SERIALIZE_METHODS;
 
     template <typename Stream, typename Operation>
     inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
         READWRITE(satoshis);
         READWRITE(script);
         READWRITE(blockHeight);
     }
 
     CAddressUnspentValue(CAmount sats, CScript scriptPubKey, int height) {
         satoshis = sats;
         script = scriptPubKey;
         blockHeight = height;
     }
 
     CAddressUnspentValue() {
         SetNull();
     }
 
     void SetNull() {
         satoshis = -1;
         script.clear();
         blockHeight = 0;
     }
 
     bool IsNull() const {
         return (satoshis == -1);
     }
 };
 
 struct CAddressIndexKey {
     unsigned int type;
     uint160 hashBytes;
     int blockHeight;
     unsigned int txindex;
     uint256 txhash;
     size_t index;
     bool spending;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 66;
     }
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata8(s, type);
         hashBytes.Serialize(s, nType, nVersion);
         // Heights are stored big-endian for key sorting in LevelDB
         ser_writedata32be(s, blockHeight);
         ser_writedata32be(s, txindex);
         txhash.Serialize(s, nType, nVersion);
         ser_writedata32(s, index);
         char f = spending;
         ser_writedata8(s, f);
     }
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         type = ser_readdata8(s);
         hashBytes.Unserialize(s, nType, nVersion);
         blockHeight = ser_readdata32be(s);
         txindex = ser_readdata32be(s);
         txhash.Unserialize(s, nType, nVersion);
         index = ser_readdata32(s);
         char f = ser_readdata8(s);
         spending = f;
     }
 
     CAddressIndexKey(unsigned int addressType, uint160 addressHash, int height, int blockindex,
                      uint256 txid, size_t indexValue, bool isSpending) {
         type = addressType;
         hashBytes = addressHash;
         blockHeight = height;
         txindex = blockindex;
         txhash = txid;
         index = indexValue;
         spending = isSpending;
     }
 
     CAddressIndexKey() {
         SetNull();
     }
 
     void SetNull() {
         type = 0;
         hashBytes.SetNull();
         blockHeight = 0;
         txindex = 0;
         txhash.SetNull();
         index = 0;
         spending = false;
     }
 
 };
 
 struct CAddressIndexIteratorKey {
     unsigned int type;
     uint160 hashBytes;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 21;
     }
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata8(s, type);
         hashBytes.Serialize(s, nType, nVersion);
     }
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         type = ser_readdata8(s);
         hashBytes.Unserialize(s, nType, nVersion);
     }
 
     CAddressIndexIteratorKey(unsigned int addressType, uint160 addressHash) {
         type = addressType;
         hashBytes = addressHash;
     }
 
     CAddressIndexIteratorKey() {
         SetNull();
     }
 
     void SetNull() {
         type = 0;
         hashBytes.SetNull();
     }
 };
 
 struct CAddressIndexIteratorHeightKey {
     unsigned int type;
     uint160 hashBytes;
     int blockHeight;
 
     size_t GetSerializeSize(int nType, int nVersion) const {
         return 25;
     }
     template<typename Stream>
     void Serialize(Stream& s, int nType, int nVersion) const {
         ser_writedata8(s, type);
         hashBytes.Serialize(s, nType, nVersion);
         ser_writedata32be(s, blockHeight);
     }
     template<typename Stream>
     void Unserialize(Stream& s, int nType, int nVersion) {
         type = ser_readdata8(s);
         hashBytes.Unserialize(s, nType, nVersion);
         blockHeight = ser_readdata32be(s);
     }
 
     CAddressIndexIteratorHeightKey(unsigned int addressType, uint160 addressHash, int height) {
         type = addressType;
         hashBytes = addressHash;
         blockHeight = height;
     }
 
     CAddressIndexIteratorHeightKey() {
         SetNull();
     }
 
     void SetNull() {
         type = 0;
         hashBytes.SetNull();
         blockHeight = 0;
     }
 };


struct CDiskTxPos : public CDiskBlockPos
{
    unsigned int nTxOffset; // after header

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CDiskBlockPos*)this);
        READWRITE(VARINT(nTxOffset));
    }

    CDiskTxPos(const CDiskBlockPos &blockIn, unsigned int nTxOffsetIn) : CDiskBlockPos(blockIn.nFile, blockIn.nPos), nTxOffset(nTxOffsetIn) {
    }

    CDiskTxPos() {
        SetNull();
    }

    void SetNull() {
        CDiskBlockPos::SetNull();
        nTxOffset = 0;
    }
};


CAmount GetMinRelayFee(const CTransaction& tx, unsigned int nBytes, bool fAllowFree);

/**
 * Check transaction inputs, and make sure any
 * pay-to-script-hash transactions are evaluating IsStandard scripts
 *
 * Why bother? To avoid denial-of-service attacks; an attacker
 * can submit a standard HASH... OP_EQUAL transaction,
 * which will get accepted into blocks. The redemption
 * script can be anything; an attacker could use a very
 * expensive-to-check-upon-redemption script like:
 *   DUP CHECKSIG DROP ... repeated 100 times... OP_1
 */

/**
 * Check for standard transaction types
 * @param[in] mapInputs    Map of previous transactions that have outputs we're spending
 * @return True if all inputs (scriptSigs) use only standard transaction forms
 */
bool AreInputsStandard(const CTransaction& tx, const CCoinsViewCache& mapInputs);

/**
 * Count ECDSA signature operations the old-fashioned (pre-0.6) way
 * @return number of sigops this transaction's outputs will produce when spent
 * @see CTransaction::FetchInputs
 */
unsigned int GetLegacySigOpCount(const CTransaction& tx);

/**
 * Count ECDSA signature operations in pay-to-script-hash inputs.
 *
 * @param[in] mapInputs Map of previous transactions that have outputs we're spending
 * @return maximum number of sigops required to validate this transaction's inputs
 * @see CTransaction::FetchInputs
 */
unsigned int GetP2SHSigOpCount(const CTransaction& tx, const CCoinsViewCache& mapInputs);


/**
 * Check whether all inputs of this transaction are valid (no double spends, scripts & sigs, amounts)
 * This does not modify the UTXO set. If pvChecks is not NULL, script checks are pushed onto it
 * instead of being performed inline.
 */
bool ContextualCheckInputs(const CTransaction& tx, CValidationState &state, const CCoinsViewCache &view, bool fScriptChecks,
                           unsigned int flags, bool cacheStore, const Consensus::Params& consensusParams,
                           std::vector<CScriptCheck> *pvChecks = NULL);

/** Apply the effects of this transaction on the UTXO set represented by view */
void UpdateCoins(const CTransaction& tx, CValidationState &state, CCoinsViewCache &inputs, int nHeight);

/** Context-independent validity checks */
bool CheckTransaction(const CTransaction& tx, CValidationState& state, libzcash::ProofVerifier& verifier);
bool CheckTransactionWithoutProofVerification(const CTransaction& tx, CValidationState &state);
bool CheckJoinSplitSigs(const CTransaction& tx, CValidationState &state, const unsigned int flags);

/** Check for standard transaction types
 * @return True if all outputs (scriptPubKeys) use only standard transaction forms
 */
bool IsStandardTx(const CTransaction& tx, std::string& reason);

/**
 * Check if transaction is final and can be included in a block with the
 * specified height and time. Consensus critical.
 */
bool IsFinalTx(const CTransaction &tx, int nBlockHeight, int64_t nBlockTime);

/**
 * Check if transaction will be final in the next block to be created.
 *
 * Calls IsFinalTx() with current block height and appropriate block time.
 *
 * See consensus/consensus.h for flag definitions.
 */
bool CheckFinalTx(const CTransaction &tx, int flags = -1);

/**
 * Closure representing one script verification
 * Note that this stores references to the spending transaction
 */
class CScriptCheck
{
private:
    CScript scriptPubKey;
    const CTransaction *ptxTo;
    unsigned int nIn;
    unsigned int nFlags;
    bool cacheStore;
    ScriptError error;

public:
    CScriptCheck(): ptxTo(0), nIn(0), nFlags(0), cacheStore(false), error(SCRIPT_ERR_UNKNOWN_ERROR) {}
    CScriptCheck(const CCoins& txFromIn, const CTransaction& txToIn, unsigned int nInIn, unsigned int nFlagsIn, bool cacheIn) :
        scriptPubKey(txFromIn.vout[txToIn.vin[nInIn].prevout.n].scriptPubKey),
        ptxTo(&txToIn), nIn(nInIn), nFlags(nFlagsIn), cacheStore(cacheIn), error(SCRIPT_ERR_UNKNOWN_ERROR) { }

    bool operator()();

    void swap(CScriptCheck &check) {
        scriptPubKey.swap(check.scriptPubKey);
        std::swap(ptxTo, check.ptxTo);
        std::swap(nIn, check.nIn);
        std::swap(nFlags, check.nFlags);
        std::swap(cacheStore, check.cacheStore);
        std::swap(error, check.error);
    }

    ScriptError GetScriptError() const { return error; }
};

bool GetTimestampIndex(const unsigned int &high, const unsigned int &low, const bool fActiveOnly, std::vector<std::pair<uint256, unsigned int> > &hashes);
 bool GetSpentIndex(CSpentIndexKey &key, CSpentIndexValue &value);
 bool GetAddressIndex(uint160 addressHash, int type,
                      std::vector<std::pair<CAddressIndexKey, CAmount> > &addressIndex,
                      int start = 0, int end = 0);
 bool GetAddressUnspent(uint160 addressHash, int type,
                        std::vector<std::pair<CAddressUnspentKey, CAddressUnspentValue> > &unspentOutputs);

						
/** Functions for disk access for blocks */
bool WriteBlockToDisk(CBlock& block, CDiskBlockPos& pos, const CMessageHeader::MessageStartChars& messageStart);
bool ReadBlockFromDisk(CBlock& block, const CDiskBlockPos& pos
#ifdef FORK_CB_INPUT
        , int nHeight = -1
#endif
);
bool ReadBlockFromDisk(CBlock& block, const CBlockIndex* pindex);


/** Functions for validating blocks and updating the block tree */

/** Undo the effects of this block (with given index) on the UTXO set represented by coins.
 *  In case pfClean is provided, operation will try to be tolerant about errors, and *pfClean
 *  will be true if no problems were found. Otherwise, the return value will be false in case
 *  of problems. Note that in any case, coins may be modified. */
bool DisconnectBlock(CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& coins, bool* pfClean = NULL);

/** Apply the effects of this block (with given index) on the UTXO set represented by coins */
bool ConnectBlock(const CBlock& block, CValidationState& state, CBlockIndex* pindex, CCoinsViewCache& coins, bool fJustCheck = false);

/** Context-independent validity checks */
bool CheckBlockHeader(const CBlockHeader& block, CValidationState& state, bool fCheckPOW = true);

bool CheckBlock(const CBlock& block, CValidationState& state,
                libzcash::ProofVerifier& verifier,
                bool fCheckPOW = true, bool fCheckMerkleRoot = true);

/** Context-dependent validity checks */
bool ContextualCheckBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex *pindexPrev);
bool ContextualCheckBlock(const CBlock& block, CValidationState& state, CBlockIndex *pindexPrev);

/** Check a block is completely valid from start to finish (only works on top of our current best block, with cs_main held) */
bool TestBlockValidity(CValidationState &state, const CBlock& block, CBlockIndex *pindexPrev, bool fCheckPOW = true, bool fCheckMerkleRoot = true);

/**
 * Store block on disk.
 * JoinSplit proofs are never verified, because:
 * - AcceptBlock doesn't perform script checks either.
 * - The only caller of AcceptBlock verifies JoinSplit proofs elsewhere.
 * If dbp is non-NULL, the file is known to already reside on disk
 */
bool AcceptBlock(CBlock& block, CValidationState& state, CBlockIndex **pindex, bool fRequested, CDiskBlockPos* dbp
#ifdef FORK_CB_INPUT
        , bool fCalledFromMiner = false
#endif
    );
bool AcceptBlockHeader(const CBlockHeader& block, CValidationState& state, CBlockIndex **ppindex= NULL);



class CBlockFileInfo
{
public:
    unsigned int nBlocks;      //! number of blocks stored in file
    unsigned int nSize;        //! number of used bytes of block file
    unsigned int nUndoSize;    //! number of used bytes in the undo file
    unsigned int nHeightFirst; //! lowest height of block in file
    unsigned int nHeightLast;  //! highest height of block in file
    uint64_t nTimeFirst;         //! earliest time of block in file
    uint64_t nTimeLast;          //! latest time of block in file

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(VARINT(nBlocks));
        READWRITE(VARINT(nSize));
        READWRITE(VARINT(nUndoSize));
        READWRITE(VARINT(nHeightFirst));
        READWRITE(VARINT(nHeightLast));
        READWRITE(VARINT(nTimeFirst));
        READWRITE(VARINT(nTimeLast));
    }

     void SetNull() {
         nBlocks = 0;
         nSize = 0;
         nUndoSize = 0;
         nHeightFirst = 0;
         nHeightLast = 0;
         nTimeFirst = 0;
         nTimeLast = 0;
     }

     CBlockFileInfo() {
         SetNull();
     }

     std::string ToString() const;

     /** update statistics (does not update nSize) */
     void AddBlock(unsigned int nHeightIn, uint64_t nTimeIn) {
         if (nBlocks==0 || nHeightFirst > nHeightIn)
             nHeightFirst = nHeightIn;
         if (nBlocks==0 || nTimeFirst > nTimeIn)
             nTimeFirst = nTimeIn;
         nBlocks++;
         if (nHeightIn > nHeightLast)
             nHeightLast = nHeightIn;
         if (nTimeIn > nTimeLast)
             nTimeLast = nTimeIn;
     }
};

/** RAII wrapper for VerifyDB: Verify consistency of the block and coin databases */
class CVerifyDB {
public:
    CVerifyDB();
    ~CVerifyDB();
    bool VerifyDB(CCoinsView *coinsview, int nCheckLevel, int nCheckDepth);
};

/** Find the last common block between the parameter chain and a locator. */
CBlockIndex* FindForkInGlobalIndex(const CChain& chain, const CBlockLocator& locator);

/** Mark a block as invalid. */
bool InvalidateBlock(CValidationState& state, CBlockIndex *pindex);

/** Remove invalidity status from a block and its descendants. */
bool ReconsiderBlock(CValidationState& state, CBlockIndex *pindex);

/** The currently-connected chain of blocks. */
extern CChain chainActive;

/** Global variable that points to the active CCoinsView (protected by cs_main) */
extern CCoinsViewCache *pcoinsTip;

/** Global variable that points to the active block tree (protected by cs_main) */
extern CBlockTreeDB *pblocktree;

/**
 * Return the spend height, which is one more than the inputs.GetBestBlock().
 * While checking, GetBestBlock() refers to the parent block. (protected by cs_main)
 * This is also true for mempool checks.
 */
int GetSpendHeight(const CCoinsViewCache& inputs);

extern VersionBitsCache versionbitscache;

/**
 * Determine what nVersion a new block should use.
 */
int32_t ComputeBlockVersion(const CBlockIndex* pindexPrev, const Consensus::Params& params);

namespace Consensus {
bool CheckTxInputs(const CTransaction& tx, CValidationState& state, const CCoinsViewCache& inputs, int nSpendHeight, const Consensus::Params& consensusParams);
}

#ifdef FORK_CB_INPUT
#define FORK_COINBASE_PER_BLOCK 10000

extern std::string forkUtxoPath;
extern int64_t forkStartHeight;
extern int64_t forkHeightRange;
extern int64_t forkCBPerBlock;
extern uint256 forkExtraHashSentinel;

std::string GetUTXOFileName(int nHeight);

//ex: forkStartHeight = 300 000; forkHeightRange = 65K
//A. for miner:
//   1.   Current height = 299 999; the next block to create 300 000
//                  nHeight is 300 000 - return false
//   2.   Current height = 300 000; the next block to create 300 001
//                  nHeight is 300 001 - return true - file to use utxo-00001.bin
//      ...
//
//   n-1. Current height = 364 999; the next block to create 365 000
//                  nHeight is 365 000 - return true - file to use utxo-65000.bin
//   n.   Current height = 365 000; the next block to create 365 001
//                  nHeight is 365 001 - return false
//
//  fork blocks 300001 - 365000
//
//B. for acceptblock:
//   1.   Current height = 299 999
//                  nHeight is 299 999 - return false - no verification
//   2.   Current height = 300 000
//                  nHeight is 300 000 - return false - no verification
//   3.   Current height = 300 001
//                  nHeight is 300 001 - return true - verify with file utxo-00001.bin
//      ...
//
//   n.   Current height = 365 000
//                  nHeight is 365 000 - return true - verify with file utxo-65000.bin
//   n+1. Current height = 365 001
//                  nHeight is 365 001 - return false - no verification
//
inline bool isForkBlock(int nHeight)
{
    return (nHeight > forkStartHeight && nHeight <= forkStartHeight + forkHeightRange);
}

inline bool looksLikeForkBlockHeader(const CBlockHeader& header)
{
    return header.hashReserved == forkExtraHashSentinel;
}

inline uint64_t bytes2uint64(char *array)
{
    uint64_t x =
    static_cast<uint64_t>(array[0])       & 0x00000000000000ff |
    static_cast<uint64_t>(array[1]) << 8  & 0x000000000000ff00 |
    static_cast<uint64_t>(array[2]) << 16 & 0x0000000000ff0000 |
    static_cast<uint64_t>(array[3]) << 24 & 0x00000000ff000000 |
    static_cast<uint64_t>(array[4]) << 32 & 0x000000ff00000000 |
    static_cast<uint64_t>(array[5]) << 40 & 0x0000ff0000000000 |
    static_cast<uint64_t>(array[6]) << 48 & 0x00ff000000000000 |
    static_cast<uint64_t>(array[7]) << 56 & 0xff00000000000000;
    return x;
}
#endif

inline bool isForkEnabled(int nHeight)
{
    return nHeight > forkStartHeight;
}

extern uint256 hashPid;

#endif // BITCOIN_MAIN_H
