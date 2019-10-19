#!/usr/bin/env python2
# Copyright (c) 2018 The Zcash developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal, initialize_chain_clean, \
    start_nodes, connect_nodes_bi, wait_and_assert_operationid_status
from test_framework.authproxy import JSONRPCException

from decimal import Decimal

class WalletGrothTxTest (BitcoinTestFramework):

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 4)

    def setup_network(self, split=False):
        self.nodes = start_nodes(4, self.options.tmpdir, extra_args=[["-debug=zrpcunsafe", "-txindex"]] * 4 )
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        connect_nodes_bi(self.nodes,0,3)
        self.is_network_split=False
        self.sync_all()

    def run_test (self):
        BLOCK_REWARD_BELOW_100 = 50.00
        
        self.nodes[0].generate(100)
        self.sync_all()
        self.nodes[1].generate(98)
        self.sync_all()
        # Node 0 has reward from blocks 1 to 98 which are spendable.

        taddr0 = self.nodes[0].getnewaddress()
        taddr1 = self.nodes[1].getnewaddress()
        taddr2 = self.nodes[2].getnewaddress()
        zaddr2 = self.nodes[2].z_getnewaddress()
        taddr3 = self.nodes[3].getnewaddress()
        zaddr3 = self.nodes[3].z_getnewaddress()


        # Node 0 sends transparent funds to Node 2
        tsendamount = Decimal('1.0')
        txid_transparent = self.nodes[0].sendtoaddress(taddr2, tsendamount)
        self.sync_all()

        # Node 2 sends the zero-confirmation transparent funds to Node 1 using z_sendmany
        recipients = []
        recipients.append({"address":taddr1, "amount": Decimal('0.5')})
        myopid = self.nodes[2].z_sendmany(taddr2, recipients, 0)
        txid_zsendmany = wait_and_assert_operationid_status(self.nodes[2], myopid)

        # Node 0 shields to Node 2, a coinbase utxo of value 10.0 less fee 0.00010000
        zsendamount = Decimal(BLOCK_REWARD_BELOW_100) - Decimal('0.0001')
        recipients = []
        recipients.append({"address":zaddr2, "amount": zsendamount})
        myopid = self.nodes[0].z_sendmany(taddr0, recipients)
        txid_shielded = wait_and_assert_operationid_status(self.nodes[0], myopid)

        self.sync_all()
        self.nodes[0].generate(1)
        self.sync_all()

        # Verify balance
        assert_equal(self.nodes[1].z_getbalance(taddr1), Decimal('0.5'))
        assert_equal(self.nodes[2].getbalance(), Decimal('0.4999'))
        assert_equal(self.nodes[2].z_getbalance(zaddr2), zsendamount)

        # Verify transaction versions are 1 or 2 (intended for Sprout)
        result = self.nodes[0].getrawtransaction(txid_transparent, 1)
        assert_equal(result["version"], 1)
        result = self.nodes[0].getrawtransaction(txid_zsendmany, 1)
        assert_equal(result["version"], 1)
        result = self.nodes[0].getrawtransaction(txid_shielded, 1)
        assert_equal(result["version"], 2)
 
        # Node 0 sends transparent funds to Node 3
        tsendamount = Decimal('1.0')
        txid_transparent = self.nodes[0].sendtoaddress(taddr3, tsendamount)
        self.sync_all()

        # Node 3 sends the zero-confirmation transparent funds to Node 1 using z_sendmany
        recipients = []
        recipients.append({"address":taddr1, "amount": Decimal('0.5')})
        myopid = self.nodes[3].z_sendmany(taddr3, recipients, 0)
        txid_zsendmany = wait_and_assert_operationid_status(self.nodes[3], myopid)

        # Node 0 shields to Node 3, a coinbase utxo of value 10.0 less fee 0.00010000
        zsendamount = Decimal(BLOCK_REWARD_BELOW_100) - Decimal('0.0001')
        recipients = []
        recipients.append({"address":zaddr3, "amount": zsendamount})
        myopid = self.nodes[0].z_sendmany(taddr0, recipients)
        txid_shielded = wait_and_assert_operationid_status(self.nodes[0], myopid)

        # Mine the first Groth block
        self.sync_all()
        self.nodes[0].generate(1)
        self.sync_all()

        # Verify balance
        assert_equal(self.nodes[1].z_getbalance(taddr1), Decimal('1.0'))
        assert_equal(self.nodes[3].getbalance(), Decimal('0.4999'))
        assert_equal(self.nodes[3].z_getbalance(zaddr3), zsendamount)

        # Verify transaction version is 3 (intended for Groth)
        result = self.nodes[0].getrawtransaction(txid_transparent, 1)
        assert_equal(result["version"], 1)

        result = self.nodes[0].getrawtransaction(txid_zsendmany, 1)
        assert_equal(result["version"], 1)

        result = self.nodes[0].getrawtransaction(txid_shielded, 1)
        assert_equal(result["version"], -3)
        

if __name__ == '__main__':
    WalletGrothTxTest().main()
