#!/usr/bin/env python2
# Copyright (c) 2017 The Zcash developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import BitcoinTestFramework
from test_framework.authproxy import JSONRPCException
from test_framework.util import assert_equal, initialize_chain_clean, \
    start_node, connect_nodes_bi, sync_blocks

import sys
import time
from decimal import Decimal

class WalletShieldCoinbaseTest (BitcoinTestFramework):

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 4)

    def setup_network(self, split=False):
        args = ['-regtestprotectcoinbase', '-debug=zrpcunsafe']
        self.nodes = []
        self.nodes.append(start_node(0, self.options.tmpdir, args))
        self.nodes.append(start_node(1, self.options.tmpdir, args))
        args2 = ['-regtestprotectcoinbase', '-debug=zrpcunsafe', "-mempooltxinputlimit=7"]
        self.nodes.append(start_node(2, self.options.tmpdir, args2))
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.is_network_split=False
        self.sync_all()

    # Returns txid if operation was a success or None
    def wait_and_assert_operationid_status(self, nodeid, myopid, in_status='success', in_errormsg=None):
        print('waiting for async operation {}'.format(myopid))
        opids = []
        opids.append(myopid)
        timeout = 300
        status = None
        errormsg = None
        txid = None
        for x in xrange(1, timeout):
            results = self.nodes[nodeid].z_getoperationresult(opids)
            if len(results)==0:
                time.sleep(1)
            else:
                status = results[0]["status"]
                if status == "failed":
                    errormsg = results[0]['error']['message']
                elif status == "success":
                    txid = results[0]['result']['txid']
                break
        # print('...returned status: {}'.format(status))
        assert_equal(in_status, status)
        if errormsg is not None:
            assert(in_errormsg is not None)
            assert_equal(in_errormsg in errormsg, True)
            # print('...returned error: {}'.format(errormsg))
        return txid

    def run_test (self):
        print "Mining blocks..."

        self.nodes[0].generate(1)
        do_not_shield_taddr = self.nodes[0].getnewaddress()

        self.nodes[0].generate(4)
        walletinfo = self.nodes[0].getwalletinfo()
        assert_equal(walletinfo['immature_balance'], 250.00)
        assert_equal(walletinfo['balance'], 0)
        self.sync_all()
        self.nodes[2].generate(1)
        self.nodes[2].getnewaddress()
        self.nodes[2].generate(1)
        self.nodes[2].getnewaddress()
        self.nodes[2].generate(1)
        self.sync_all()
        self.nodes[1].generate(101)
        self.sync_all()
        assert_equal(self.nodes[0].getbalance(), 250.00)
        assert_equal(self.nodes[1].getbalance(), 50.00)
        assert_equal(self.nodes[2].getbalance(), 150.00)

        # Prepare to send taddr->zaddr
        mytaddr = self.nodes[0].getnewaddress()
        myzaddr = self.nodes[0].z_getnewaddress()

        # Shielding will fail when trying to spend from watch-only address
        self.nodes[2].importaddress(mytaddr)
        try:
            self.nodes[2].z_shieldcoinbase(mytaddr, myzaddr)
        except JSONRPCException,e:
            errorString = e.error['message']
        assert_equal("Could not find any coinbase funds to shield" in errorString, True)

        # Shielding will fail because fee is negative
        try:
            self.nodes[0].z_shieldcoinbase("*", myzaddr, -1)
        except JSONRPCException,e:
            errorString = e.error['message']
        assert_equal("Amount out of range" in errorString, True)

        # Shielding will fail because fee is larger than MAX_MONEY
        try:
            self.nodes[0].z_shieldcoinbase("*", myzaddr, Decimal('21000000.00000001'))
        except JSONRPCException,e:
            errorString = e.error['message']
        assert_equal("Amount out of range" in errorString, True)

        # Shielding will fail because fee is larger than sum of utxos
        try:
            self.nodes[0].z_shieldcoinbase("*", myzaddr, 999)
        except JSONRPCException,e:
            errorString = e.error['message']
        assert_equal("Insufficient coinbase funds" in errorString, True)

        # Shield coinbase utxos from node 0 of value 40, standard fee of 0.00010000
        result = self.nodes[0].z_shieldcoinbase(mytaddr, myzaddr)
        mytxid = self.wait_and_assert_operationid_status(0, result['opid'])
        self.sync_all()
        self.nodes[1].generate(1)
        self.sync_all()

        # Confirm balances and that do_not_shield_taddr containing funds of 10 was left alone
        assert_equal(self.nodes[0].getbalance(), 50.00)
        assert_equal(self.nodes[0].z_getbalance(do_not_shield_taddr), Decimal('50.00'))
        assert_equal(self.nodes[0].z_getbalance(myzaddr), Decimal('199.99990000'))
        assert_equal(self.nodes[1].getbalance(), 100.00)
        assert_equal(self.nodes[2].getbalance(), 150.00)

        # Shield coinbase utxos from any node 2 taddr, and set fee to 0
        result = self.nodes[2].z_shieldcoinbase("*", myzaddr, 0)
        mytxid = self.wait_and_assert_operationid_status(2, result['opid'])
        self.sync_all()
        self.nodes[1].generate(1)
        self.sync_all()

        assert_equal(self.nodes[0].getbalance(), 50.00)
        assert_equal(self.nodes[0].z_getbalance(myzaddr), Decimal('349.9999'))
        assert_equal(self.nodes[1].getbalance(), 150.00)
        assert_equal(self.nodes[2].getbalance(), 0)

        # Generate 800 coinbase utxos on node 0, and 20 coinbase utxos on node 2
        self.nodes[0].generate(800)
        self.sync_all()
        self.nodes[2].generate(20)
        self.sync_all()
        self.nodes[1].generate(100)
        self.sync_all()
        mytaddr = self.nodes[0].getnewaddress()

        # Shielding the 800 utxos will occur over two transactions, since max tx size is 100,000 bytes.
        # We don't verify shieldingValue as utxos are not selected in any specific order, so value can change on each test run.
        result = self.nodes[0].z_shieldcoinbase(mytaddr, myzaddr, 0)
        assert_equal(result["shieldingUTXOs"], Decimal('662'))
        assert_equal(result["remainingUTXOs"], Decimal('138'))
        remainingValue = result["remainingValue"]
        opid1 = result['opid']

        # Verify that utxos are locked (not available for selection) by queuing up another shielding operation
        result = self.nodes[0].z_shieldcoinbase(mytaddr, myzaddr)
        assert_equal(result["shieldingValue"], Decimal(remainingValue))
        assert_equal(result["shieldingUTXOs"], Decimal('138'))
        assert_equal(result["remainingValue"], Decimal('0'))
        assert_equal(result["remainingUTXOs"], Decimal('0'))
        opid2 = result['opid']

        # wait for both aysnc operations to complete
        self.wait_and_assert_operationid_status(0, opid1)
        self.wait_and_assert_operationid_status(0, opid2)

        # sync_all() invokes sync_mempool() but node 2's mempool limit will cause tx1 and tx2 to be rejected.
        # So instead, we sync on blocks, and after a new block is generated, all nodes will have an empty mempool.
        sync_blocks(self.nodes)
        self.nodes[1].generate(1)
        self.sync_all()

        # Verify maximum number of utxos which node 2 can shield is limited by option -mempooltxinputlimit
        mytaddr = self.nodes[2].getnewaddress()
        result = self.nodes[2].z_shieldcoinbase(mytaddr, myzaddr, 0)
        assert_equal(result["shieldingUTXOs"], Decimal('7'))
        assert_equal(result["remainingUTXOs"], Decimal('13'))
        mytxid = self.wait_and_assert_operationid_status(2, result['opid'])
        self.sync_all()
        self.nodes[1].generate(1)
        self.sync_all()

if __name__ == '__main__':
    WalletShieldCoinbaseTest().main()
