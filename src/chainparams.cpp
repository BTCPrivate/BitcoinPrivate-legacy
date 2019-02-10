
// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2016-2017 The Zcash developers
// Copyright (c) 2018 The Bitcoin Private developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "main.h"
#include "crypto/equihash.h"

#include "util.h"
#include "utilstrencodings.h"

#include <assert.h>

#include <boost/assign/list_of.hpp>

#include "base58.h"

using namespace std;

#include "chainparamsseeds.h"

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

class CMainParams : public CChainParams {
public:
    CMainParams() {
        strNetworkID = "main";
        strCurrencyUnits = "BTCP";
        consensus.fCoinbaseMustBeProtected = true;
        consensus.nSubsidySlowStartInterval = 2;
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        consensus.prePowLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPowTargetSpacing = 2.5 * 60;
        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.nPowDifficultyBombHeight = 600000;

        consensus.nRuleChangeActivationThreshold = 1916; // 95% of 2016
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008
        consensus.nUnmovedBurnHeight = 480000;
        consensus.zResetHeight = 455500;

        pchMessageStart[0] = 0xa8;
        pchMessageStart[1] = 0xea;
        pchMessageStart[2] = 0xa2;
        pchMessageStart[3] = 0xcd;
        vAlertPubKey = ParseHex("04f2cd746e629ffd320a81287474c98c2ad15d15b0a210b0144edcd8f3e1301c6311fd751fa34ba17d88090374cfec7cd9aaca55a5a0c4456511acc01b922005de");
        nDefaultPort = 7933;
        nMaxTipAge = 24 * 60 * 60;
        nPruneAfterHeight = 100000;
        const size_t N = 200, K = 9;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;

        /**
         * Build the genesis block. Note that the output of its generation
         * transaction cannot be spent since it did not originally exist in the
         * database (and is in any case of zero value).
         *
         * >>> from pyblake2 import blake2s
         * >>> 'Zclassic' + blake2s(b'No taxation without representation. BTC #437541 - 00000000000000000397f175a94dd3f530b957182eb2a9f7b79a44a94a5e0450').hexdigest()
         */
        const char* pszTimestamp = "Zclassic860413afe207aa173afee4fcfa9166dc745651c754a41ea8f155646f5aa828ac";
        CMutableTransaction txNew;
        txNew.vin.resize(1);
        txNew.vout.resize(1);
        txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
        txNew.vout[0].nValue = 0;
        txNew.vout[0].scriptPubKey = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
        genesis.vtx.push_back(txNew);
        genesis.hashPrevBlock.SetNull();
        genesis.hashMerkleRoot = genesis.BuildMerkleTree();
        genesis.nVersion = 4;
        genesis.nTime    = 1478403829;
        genesis.nBits    = 0x1f07ffff;
        genesis.nNonce   = uint256S("0x000000000000000000000000000000000000000000000000000000000000021d");
        genesis.nSolution = ParseHex("009aaa951ca873376788d3002918d956e371bdf03c1afcfd8eea17867b5480d2e59a2a4dd52ed0d091af0c0909aa66ce2da97266926a9ea69b9ccca389bc120d9c4dbbae727ab9d6dfd1cd847df0ef0cc9bc989f11bdd6522429c15957daa3c5a2612522ded69857c148c0638611a19287599b47683c714b5774d0fcb1341cf4fc3a546a2441a19f02a55c6f9775749e57783b2abd5b25d41753d2f60892bbb4c3173d7787dbf5e50267324db218a14dd65f71bb02cf2566d3201800f866701db8c221424b75c639de58e7e40705157ae7d10da708ec2b9e71b9bc1ad34854a7bdf58d93766b6e291d3b545fa1f785a1a9829eccd525d16856f4317f0449d5c3516736f1e564f17690f13d3c939ad5516f1db70194902c20afd939168037fa404ec962dfbe752f79ac87a2cc3fd07bcd94d1975b1849cc739c0bc144ae4e75eda1bbed5b5ef8f65966257ec7b1fc6bb600e12e1c65c8c13a505f35dd363e07b6238211a0e502e36db5a620310b544360dd9b4a6cedabc34eeb530139daad50d4a5b6eaf4d50be4ba10e970ce984fb705376a3b0b4bf3f3778600f14e739e04406106f707085ab87ca70598c032b6717a54a9fd8ef72fdd78fb41fa9d45ad685caf77e0fc42e8e644634c24bc972f3ab0e3f0345854eda624045feb6bc9d20b5b1fc6903ebc64026e51da598c0d8711c452131a8fd2bbe01403af20e5db88afcd53b6107f001dae78b548d6a1581baca15359de83e54e75d8fc6374ca1edec17a9f4b06931162f9952575c5c3fb5dfc70a0f793049e781926daaafd4f4d330cf7d5635af1541f0d29e709a37c088d6d2e7aa09d15dfb9c2ae6c1ce661e85e9d89772eb47cfea00c621b66faf8a48cfa970b898dbd77b14e7bf44b742c00f76d2435f949f027132adb1e974551488f988e9fe379a0f86538ee59e26637a3d50bf400c7f52aa9457d77c3eb426628bb17909b26a6820d0772d4c6f74472f635e4c6e72272ce01fc475df69e10371457c55e0fbdf3a392850b9924da9c9a55792325c4318562593f0df8d39559065be03a22b1b6c21206aa1958a0d33257d89b74dea42a11aabf8eddbfe6136ab649744b704eb3e3d473654b588927dd9f486c1cd02639cf656ccbf2c4869c2ed1f2ba4ec55e69a42d5af6b3605a0cdf987734727c6fc1c1489870fb300139328c4d12eb6f5e8309cc09f5f3c29ab0957374113931ec9a56e7579446f12faacda9bd50899a17bd0f78e89ed70a723fdadfb1f4bc3317c8caa32757901604fb79ae48e22251c3b1691125ec5a99fabdf62b015bc817e1c30c06565a7071510b014058a77856a150bf86ab0c565b8bbbed159e2fb862c6215752bf3f0563e2bbbf23b0dbfb2de21b366b7e4cda212d69502643ca1f13ce362eef7435d60530b9999027dd39cd01fd8e064f1ccf6b748a2739707c9f76a041f82d3e046a9c184d83396f1f15b5a11eddb2baff40fc7b410f0c43e36ac7d8ff0204219abe4610825191fbb2be15a508c839259bfd6a4c5204c779fad6c23bbd37f90709654a5b93c6f93b4c844be12cd6cd2200afbf600b2ae9b6c133d8cdb3a85312a6d9948213c656db4d076d2bacd10577d7624be0c684bd1e5464bb39006a524d971cd2223ae9e23dea12366355b3cc4c9f6b8104df6abd23029ac4179f718e3a51eba69e4ebeec511312c423e0755b53f72ac18ef1fb445d7ab83b0894435a4b1a9cd1b473792e0628fd40bef624b4fb6ba457494cd1137a4da9e44956143068af9db98135e6890ef589726f4f5fbd45a713a24736acf150b5fb7a4c3448465322dccd7f3458c49cf2d0ef6dd7dd2ed1f1147f4a00af28ae39a73c827a38309f59faf8970448436fbb14766a3247aac4d5c610db9a662b8cb5b3e2");

        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0007104ccda289427919efc39dc9e4d499804b7bebc22df55f8b834301260602"));
        assert(genesis.hashMerkleRoot == uint256S("0x19612bcf00ea7611d315d7f43554fa983c6e8c30cba17e52c679e0e80abf7d42"));

        vFixedSeeds.clear();
        vSeeds.clear();
        // TODO: setup a DNSSeed
        vSeeds.push_back(CDNSSeedData("btcprivate.org", "dnsseed.btcprivate.org"));
        vSeeds.push_back(CDNSSeedData("btcprivate.co", "dnsseed.btcprivate.co"));

        // guarantees the first 2 characters, when base58 encoded, are "b1"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x13,0x25};
        // guarantees the first 2 characters, when base58 encoded, are "bx"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x13,0xAF};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "zk"
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xA8};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        base58Prefixes[ZCSPENDING_KEY]     = {0xAB,0x36};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_main, pnSeed6_main + ARRAYLEN(pnSeed6_main));

        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = {
            {
                {      0, consensus.hashGenesisBlock },
                {  30000, uint256S("0x000000005c2ad200c3c7c8e627f67b306659efca1268c9bb014335fdadc0c392") },
                { 160000, uint256S("0x000000065093005a1a46ee95d6d66c2b07008220ca64dd3b3a93bbd1945480c0") },
                { 272992, uint256S("0x0102a4e1353149c9221ebd7c45b3170ea4b8a7d1cf4075b3bc50c440fb9ebdd6") },
                { 273992, uint256S("0x000822e42b2128a296b704c273a4c3f0e707b06305d2be521cf7577ef67e476a") },
                { 274992, uint256S("0x03e91186991b31044179b82e88ee06c2205e3304a525d130070153fde4aaf414") },
                { 275992, uint256S("0x02bc3f08cec58ac79b35a29161b43cd301696e6e6421d704314d08a35b2169db") },
                { 276992, uint256S("0x0046665bbb64b609935ed0fd184d6c42657dc5c36b16ed1e59ecc2fcb0508d32") },
                { 277992, uint256S("0x0111d3ffc37fb3474ed2a6e3e9bfc53ee0d7cc0413a732f19d59061700f6b842") },
                { 278458, uint256S("0x0747cecedfd30754323c5afdaef547d5e90fd50485370ebf12dd33bbac61faaa") },
                { 279500, uint256S("0x0000000082ece76130c82337903b14a2109fab2dcd2d153fcdbf48a0054a694b") },
            },
            1520105951,     // * UNIX timestamp of last checkpoint block
            55463694,       // * total number of transactions between genesis and last checkpoint
                            //   (the tx=... number in the SetBestChain debug.log lines)
            2000            // * estimated number of transactions per day after checkpoint
                            //   total number of tx / (checkpoint block height / (24 * 24))
        };

        nForkStartHeight = 272991;
        nForkHeightRange = 5467;

        nEquihashForkHeight = 600001;
        nEquihashNnew = 192;
        nEquihashKnew = 7;
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CMainParams {
public:
    CTestNetParams() {
        strNetworkID = "test";
        strCurrencyUnits = "BTCPT";
        consensus.fCoinbaseMustBeProtected = true;
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        consensus.powLimit = uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.prePowLimit = consensus.powLimit;
        consensus.nPowAveragingWindow = 1;
//        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.fPowAllowMinDifficultyBlocks = true;

        consensus.nRuleChangeActivationThreshold = 1512; // 75% for testchains
        consensus.nMinerConfirmationWindow = 2016; // nPowTargetTimespan / nPowTargetSpacing

        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 1199145601; // January 1, 2008
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 1230767999; // December 31, 2008

        consensus.nPowDifficultyBombHeight = 600000;

        consensus.nUnmovedBurnHeight = 480000;
        consensus.zResetHeight = 455500;

        pchMessageStart[0] = 0xf6;
        pchMessageStart[1] = 0x1b;
        pchMessageStart[2] = 0xf6;
        pchMessageStart[3] = 0xd6;

        vAlertPubKey = ParseHex("048679fb891b15d0cada9692047fd0ae26ad8bfb83fabddbb50334ee5bc0683294deb410be20513c5af6e7b9cec717ade82b27080ee6ef9a245c36a795ab044bb3");
        nDefaultPort = 17933;
        nPruneAfterHeight = 1000;

        //! Modify the testnet genesis block so the timestamp is valid for a later start.
        genesis.nTime = 1527600786;
        genesis.nBits = 0x2007ffff;
        genesis.nNonce = uint256S("0000000000000000000000000000000000000000000000000000000000000006");
        genesis.nSolution = ParseHex("00596d6b34675ec5b6cde00c8c6149cd57b1159aa7401864f4812f83c5ef10f6a41645890dab4bfb97bf208c5eea90106c62e2d353f4844f1a2587a4da14e327f54e910c9eaf27057d8541cdd0cc6e8281b9a9830492b85e2c49c6c34228f140b157417d7a1e6e489c0ad515a96c50466bd6b184b631c9ae162b86d2c02b1f6e2cdc0727daabb07dc2d7fb3bce35d16699984a50a69f736196077b4c65e60b25e2f81efcc83f20db0130237474c79359126d218fc3a0d97109dbbb72733a67421964202fbb907e53d0256e70e9021e086ba802609b07f52ff0cba0f840acedcd5b0e4a9b78988102a743a4a4662aefb5e8c93a5f72d9273ff5ffd98602ed491eb812930bfdc5305eca05f7d55a91d30aa50f0ea6a4ef03efc2e4904825ff6fd08e1d73dee68031a147a745dbe26be0bf9425c757a78d27ff365d8b711a879bcf2b010deb6f19310270bd6b1ef31d27ab0183d0c712ce07f77fc475897f6a3f89c68bdea23a2a1844e0ed732c0ff1ea033ec235d4dd6a3176019202ccc418e7d4103ab8c35509733d88eab8bd7b81621910ee0d4f6c0a5d6fcc55c679de391a46ee33c7e70264ec677c4605bcfb75928e99c8392110a72cd5350c93fd68f5556e78f764815656706dc9517e4f73b216eb6c0c7048dce0724482678b501339b0b3171ead3e63d7abaa6fc1b7ecb564e16fd1c3426bd27de21203e5f49b13470342a676d11317c92add495dae964b31484265498f677dccc7a33986c17635b6c55929320fee4635aa8c2637cfa693dc723bcb37ad57bfbf1434e9ac7db0d5fb5be71314ac2cb21d5f61db1ff12703f889031396ed8d75a3a95dd86415dfd263ff1131091f07bfa9ca1aea9e46b84be070abeab0fb7d333f05b5509e74ed6bb58b8e624087d71710cb651f6a9a13ddc217354ba8219e08c4e48536e7e1c065d9da950096a655dce913878ec4e53093cf5701578f2bde402d94eb2e639f55bf07efd3cdeca23a29b38fd19e121f347bf1a05578ff4d24635d4bb702691838f2cca42bda0768e59727f94648d49cdda8de424a0398f3650d47155dac50da21dad9820feac063a99a33174462296cc7375735cfd9c7c1d484c938e299c7dc3ee288143131bbab60af230d4a4187d1ebfafa2c91f29a66392509fafea3042b97c965982140eb2219773469030159a0986f1a157fb64a25aaf9c0688973e79b45dc042e52c25ea5009d90c805f04572074335c2be9547159dbc1e15859eee935165f2b7c5b3dd89acbfd3c744d56b34afe18a734ee81894a168330ec99898e2ad042a036cccd1a36ece9c724cf476d404a65c4d8b1e2cbbe52c905b12abb5db83be05efe15723001c7a031e7e65230e2e4e4590df79e816e12fc684f51b04dc56e9d7c27da9a2cd9ead17ac1e58ae6798651f2cd80369547a0bd24f43fec3613784c0cc17bef71e4af22a2384a6728cc7fec4982c568671f12f63b0dbed8d11c0f5edcbc851991334b12da00d9cb5334f18a4f71915713cde326b93c31bd5d9c1501eb22d99b4650e087676d72ecd09dd45586561a050628365937fafa52b158cf81adce6e3cdee46e9626c85f28e0f59397f435e529ac4965d30d0afb9812d72407ec5239cfb3d56d8b6077866befb41f427c7dd7ee782c34e3d983d047edb71e48befcf582262bec7670298c66e690d5112760944486fdac7be99296e047be9369f7e555f5b06587123f541f6451cfd71e57f71173c87b51c08e80b2397f8c69dc3c7528576551ccfc6f2c7501f4df00a331855ee03e9f4ed9f021626ee3d6e36793b650336e5054904ebffedf83e367082381aee40c43422d81c0f0e4a00478673864dc31086ecb661e9dafac47e3b8697f4f52aeeefaccda3fd42f6e4194ab5186b7e");
		
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x05098b06c3be62b6b7ebcb2ffee3dad6989c4b613cb0be3e09f8df0e4ec65d2e"));

        vFixedSeeds.clear();
        vSeeds.clear();

        // guarantees the first 2 characters, when base58 encoded, are "n1"
        base58Prefixes[PUBKEY_ADDRESS]     = {0x19,0x57};
        // guarantees the first 2 characters, when base58 encoded, are "nx"
        base58Prefixes[SCRIPT_ADDRESS]     = {0x19,0xE0};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "zz"
        base58Prefixes[ZCPAYMENT_ADDRRESS] = {0x16,0xC0};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        vFixedSeeds = std::vector<SeedSpec6>(pnSeed6_test, pnSeed6_test + ARRAYLEN(pnSeed6_test));

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;

        checkpointData = (Checkpoints::CCheckpointData) {
            boost::assign::map_list_of
            ( 0, consensus.hashGenesisBlock),
            genesis.nTime,
            0,
            0
        };

        nForkStartHeight = 10;
        nForkHeightRange = 0;

        nEquihashForkHeight = 5;
        nEquihashNnew = 192;
        nEquihashKnew = 7;
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CTestNetParams {
public:
    CRegTestParams() {
        strNetworkID = "regtest";
        strCurrencyUnits = "REG";
        consensus.fCoinbaseMustBeProtected = false;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nSubsidyHalvingInterval = 840000;
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f");
        consensus.prePowLimit = consensus.powLimit;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up

        consensus.nRuleChangeActivationThreshold = 108; // 75% for testchains
        consensus.nMinerConfirmationWindow = 144; // Faster than normal for regtest (144 instead of 2016)
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].bit = 28;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nStartTime = 0;
        consensus.vDeployments[Consensus::DEPLOYMENT_TESTDUMMY].nTimeout = 999999999999ULL;

        consensus.nPowDifficultyBombHeight = 600000;
        consensus.nUnmovedBurnHeight = 480000;
        consensus.zResetHeight = 455500;

        pchMessageStart[0] = 0xaa;
        pchMessageStart[1] = 0xe8;
        pchMessageStart[2] = 0x3f;
        pchMessageStart[3] = 0x5f;
        nMaxTipAge = 24 * 60 * 60;
        const size_t N = 48, K = 5;
        BOOST_STATIC_ASSERT(equihash_parameters_acceptable(N, K));
        nEquihashN = N;
        nEquihashK = K;
        genesis.nTime = 1482971059;
        genesis.nBits = 0x200f0f0f;
        genesis.nNonce = uint256S("0x0000000000000000000000000000000000000000000000000000000000000009");
        genesis.nSolution = ParseHex("05ffd6ad016271ade20cfce093959c3addb2079629f9f123c52ef920caa316531af5af3f");
        consensus.hashGenesisBlock = genesis.GetHash();
        nDefaultPort = 17944;
        assert(consensus.hashGenesisBlock == uint256S("0x0575f78ee8dc057deee78ef691876e3be29833aaee5e189bb0459c087451305a"));
        nPruneAfterHeight = 1000;

        vFixedSeeds.clear(); //! Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();  //! Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (Checkpoints::CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0x0575f78ee8dc057deee78ef691876e3be29833aaee5e189bb0459c087451305a")),
            0,
            0,
            0
        };

        nForkStartHeight = 50;
        nForkHeightRange = 0;

        nEquihashForkHeight = 100;
        nEquihashNnew = 192;
        nEquihashKnew = 7;
    }
};
static CRegTestParams regTestParams;

static CChainParams *pCurrentParams = 0;

const CChainParams &Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

CChainParams &Params(CBaseChainParams::Network network) {
    switch (network) {
        case CBaseChainParams::MAIN:
            return mainParams;
        case CBaseChainParams::TESTNET:
            return testNetParams;
        case CBaseChainParams::REGTEST:
            return regTestParams;
        default:
            assert(false && "Unimplemented network");
            return mainParams;
    }
}

void SelectParams(CBaseChainParams::Network network) {
    SelectBaseParams(network);
    pCurrentParams = &Params(network);

    // Some python qa rpc tests need to enforce the coinbase consensus rule
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestprotectcoinbase")) {
        regTestParams.SetRegTestCoinbaseMustBeProtected();
    }
}

bool SelectParamsFromCommandLine()
{
    CBaseChainParams::Network network = NetworkIdFromCommandLine();
    if (network == CBaseChainParams::MAX_NETWORK_TYPES)
        return false;

    SelectParams(network);
    return true;
}
