#!/bin/bash
set -e -o pipefail

CURDIR=$(cd $(dirname "$0"); pwd)
# Get BUILDDIR and REAL_BITCOIND
. "${CURDIR}/tests-config.sh"

export BITCOINCLI=${BUILDDIR}/qa/pull-tester/run-bitcoin-cli
export BITCOIND=${REAL_BITCOIND}

#Run the tests

testScripts=(
    'paymentdisclosure.py'
    # 'prioritisetransaction.py'                #broken - No module named pyblake2
    'wallet_treestate.py'
    'wallet_protectcoinbase.py'
    'wallet_shieldcoinbase.py'
    'wallet.py'
    'wallet_nullifiers.py'
    'wallet_1941.py'
    'wallet_grothtx.py'
    'listtransactions.py' 
    'mempool_resurrect_test.py' 
    'txn_doublespend.py'                  
    'txn_doublespend.py --mineblock'        
    'getchaintips.py' 
    'rawtransactions.py' 
    'rest.py' 
    'mempool_spendcoinbase.py'
    'mempool_coinbase_spends.py'              
    'mempool_tx_input_limit.py'               
    'httpbasics.py' 
    'zapwallettxes.py' 
    'proxy_test.py' 
    'merkle_blocks.py'
    'fundrawtransaction.py' 
    'signrawtransactions.py'
    'walletbackup.py' 
    'key_import_export.py'  
    'nodehandling.py' 
    'reindex.py' 
    'decodescript.py'
    'disablewallet.py'
    'zcjoinsplit.py'
    'zcjoinsplitdoublespend.py'              
    'zkey_import_export.py'
    # 'getblocktemplate.py'                     #disabled - due to masternode sync, and also need to be reworked to check for "coinbase_required_outputs"
    # 'bip65-cltv-p2p.py'                       #currently not supported - this test is meant to exercise BIP65 (CHECKLOCKTIMEVERIFY)
    # 'bipdersig-p2p.py'                        #currently not supported - this test is meant to exercise BIP66 (DER SIG)
    # 'p2p-masternodes.py'                      #disabled - this test not fully implemented yet
);
testScriptsExt=(
    'getblocktemplate_longpoll.py'
    'getblocktemplate_proposals.py'
    # 'pruning.py'                  # disabled for ANON. Failed because of the issue #1302 in zcash
    'forknotify.py'
    # 'hardforkdetection.py'        # disabled for ANON. Failed because of the issue #1302 in zcash
    # 'invalidateblock.py'          # disabled for ANON. Failed because of the issue #1302 in zcash
    'keypool.py'
    'receivedby.py'
    'rpcbind_test.py'
    # 'script_test.py'              # requires create_block functionality that is not implemented for zcash blocks yet
    'smartfees.py'
    'maxblocksinflight.py'
    # 'invalidblockrequest.py'      # requires create_block functionality that is not implemented for zcash blocks yet
    'p2p-versionbits-warning.py'
    # 'forknotify.py'
    # 'p2p-acceptblock.py'          # requires create_block functionality that is not implemented for zcash blocks yet
);

if [ "x$ENABLE_ZMQ" = "x1" ]; then
  testScripts+=('zmq_test.py')
fi

# disabled due to masternode sync, this test needs to be updated/reworked.
# if [ "x$ENABLE_PROTON" = "x1" ]; then
#   testScripts+=('proton_test.py')
# fi

extArg="-extended"
passOn=${@#$extArg}

successCount=0
declare -a failures

function runTestScript
{
    local testName="$1"
    shift

    echo -e "=== Running testscript ${testName} ==="

    if eval "$@"
    then
        successCount=$(expr $successCount + 1)
        echo "--- Success: ${testName} ---"
    else
        failures[${#failures[@]}]="$testName"
        echo "!!! FAIL: ${testName} !!!"
    fi

    echo
}

if [ "x${ENABLE_BITCOIND}${ENABLE_UTILS}${ENABLE_WALLET}" = "x111" ]; then
    for (( i = 0; i < ${#testScripts[@]}; i++ ))
    do
        if [ -z "$1" ] || [ "${1:0:1}" == "-" ] || [ "$1" == "${testScripts[$i]}" ] || [ "$1.py" == "${testScripts[$i]}" ]
        then
            runTestScript \
                "${testScripts[$i]}" \
                "${BUILDDIR}/qa/rpc-tests/${testScripts[$i]}" \
                --srcdir "${BUILDDIR}/src" ${passOn}
        fi
    done
    for (( i = 0; i < ${#testScriptsExt[@]}; i++ ))
    do
        if [ "$1" == $extArg ] || [ "$1" == "${testScriptsExt[$i]}" ] || [ "$1.py" == "${testScriptsExt[$i]}" ]
        then
            runTestScript \
                "${testScriptsExt[$i]}" \
                "${BUILDDIR}/qa/rpc-tests/${testScriptsExt[$i]}" \
                --srcdir "${BUILDDIR}/src" ${passOn}
        fi
    done

    echo -e "\n\nTests completed: $(expr $successCount + ${#failures[@]})"
    echo "successes $successCount; failures: ${#failures[@]}"

    if [ ${#failures[@]} -gt 0 ]
    then
        echo -e "\nFailing tests: ${failures[*]}"
        exit 1
    else
        exit 0
    fi
else
  echo "No rpc tests to run. Wallet, utils, and bitcoind must all be enabled"
fi
