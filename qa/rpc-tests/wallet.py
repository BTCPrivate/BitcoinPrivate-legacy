#!/usr/bin/env python2
# Copyright (c) 2014 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.


from test_framework.test_framework import BitcoinTestFramework
from test_framework.authproxy import JSONRPCException
from test_framework.util import assert_equal, assert_greater_than, \
    initialize_chain_clean, start_nodes, start_node, connect_nodes_bi, \
    stop_nodes, sync_blocks, sync_mempools, wait_bitcoinds

import time
from decimal import Decimal

class WalletTest (BitcoinTestFramework):

    def setup_chain(self):
        print("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 4)

    def setup_network(self, split=False):
        self.nodes = start_nodes(3, self.options.tmpdir)
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.is_network_split=False
        self.sync_all()

    def run_test (self):
        print "Mining blocks..."

        self.nodes[0].generate(4)

        walletinfo = self.nodes[0].getwalletinfo()
        assert_equal(walletinfo['immature_balance'], 50.00 * 4)
        assert_equal(walletinfo['balance'], 0)

        self.sync_all()
        self.nodes[1].generate(101)
        self.sync_all()

        assert_equal(self.nodes[0].getbalance(), 50.00 * 4)
        assert_equal(self.nodes[1].getbalance(), 50.00)
        assert_equal(self.nodes[2].getbalance(), 0)
        assert_equal(self.nodes[0].getbalance("*"), 50.00 * 4)
        assert_equal(self.nodes[1].getbalance("*"), 50.00)
        assert_equal(self.nodes[2].getbalance("*"), 0)

        # Send 21 BTC from 0 to 2 using sendtoaddress call.
        # Second transaction will be child of first, and will require a fee
        self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 52)
        self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 50.00)

        walletinfo = self.nodes[0].getwalletinfo()
        assert_equal(walletinfo['immature_balance'], 0)

        # Have node0 mine a block, thus it will collect its own fee.
        self.sync_all()
        self.nodes[0].generate(1)
        self.sync_all()

        # Have node1 generate 100 blocks (so node0 can recover the fee)
        self.nodes[1].generate(100)
        self.sync_all()

        # node0 should end up with (50.00*4 + 8.75) btc in block rewards plus fees, but
        # minus the 62.00 plus fees sent to node2
        assert_equal(self.nodes[0].getbalance(), (50.00*4 + 50.00)-102.00)
        assert_equal(self.nodes[2].getbalance(), 102.00)
        assert_equal(self.nodes[0].getbalance("*"), (50.00*4 + 50.00)-102.00)
        assert_equal(self.nodes[2].getbalance("*"), 102.00)

        # Node0 should have three unspent outputs.
        # Create a couple of transactions to send them to node2, submit them through
        # node1, and make sure both node0 and node2 pick them up properly:
        node0utxos = self.nodes[0].listunspent(1)
        assert_equal(len(node0utxos), 3)

        # Check 'generated' field of listunspent
        # Node 0: has one coinbase utxo and two regular utxos
        assert_equal(sum(int(uxto["generated"] is True) for uxto in node0utxos), 1)
        # Node 1: has 101 coinbase utxos and no regular utxos
        node1utxos = self.nodes[1].listunspent(1)
        assert_equal(len(node1utxos), 101)
        assert_equal(sum(int(uxto["generated"] is True) for uxto in node1utxos), 101)
        # Node 2: has no coinbase utxos and two regular utxos
        node2utxos = self.nodes[2].listunspent(1)
        assert_equal(len(node2utxos), 2)
        assert_equal(sum(int(uxto["generated"] is True) for uxto in node2utxos), 0)

        # create both transactions
        txns_to_send = []
        for utxo in node0utxos:
            inputs = []
            outputs = {}
            inputs.append({ "txid" : utxo["txid"], "vout" : utxo["vout"]})
            outputs[self.nodes[2].getnewaddress("")] = utxo["amount"]
            raw_tx = self.nodes[0].createrawtransaction(inputs, outputs)
            txns_to_send.append(self.nodes[0].signrawtransaction(raw_tx))

        # Have node 1 (miner) send the transactions
        self.nodes[1].sendrawtransaction(txns_to_send[0]["hex"], True)
        self.nodes[1].sendrawtransaction(txns_to_send[1]["hex"], True)
        self.nodes[1].sendrawtransaction(txns_to_send[2]["hex"], True)

        # Have node1 mine a block to confirm transactions:
        self.sync_all()
        self.nodes[1].generate(1)
        self.sync_all()

        assert_equal(self.nodes[0].getbalance(), 0)
        assert_equal(self.nodes[2].getbalance(), 50.00*5)
        assert_equal(self.nodes[0].getbalance("*"), 0)
        assert_equal(self.nodes[2].getbalance("*"), 50.00*5)

        # Send 50.00 BTC normal
        address = self.nodes[0].getnewaddress("")
        self.nodes[2].settxfee(Decimal('0.001'))
        txid = self.nodes[2].sendtoaddress(address, 50.00, "", "", False)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        # 199.999 = (50.00*3 + 50.00) - 0.001
        assert_equal(self.nodes[2].getbalance(), Decimal('199.999'))
        assert_equal(self.nodes[0].getbalance(), Decimal('50.00'))
        assert_equal(self.nodes[2].getbalance("*"), Decimal('199.999'))
        assert_equal(self.nodes[0].getbalance("*"), Decimal('50.00'))

        # Send 50.00 BTC with subtract fee from amount
        txid = self.nodes[2].sendtoaddress(address, 50.00, "", "", True)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        # 149.999 = (50.00*2 + 50.00) - 0.001
        assert_equal(self.nodes[2].getbalance(), Decimal('149.999'))
        assert_equal(self.nodes[0].getbalance(), Decimal('99.999'))
        assert_equal(self.nodes[2].getbalance("*"), Decimal('149.999'))
        assert_equal(self.nodes[0].getbalance("*"), Decimal('99.999'))

        # Sendmany 10 BTC
        self.nodes[2].sendmany("", {address: 10}, 0, "", [])
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        # 139.998 = 149.999 - 10 - 0.001
        assert_equal(self.nodes[2].getbalance(), Decimal('139.998'))
        assert_equal(self.nodes[0].getbalance(), Decimal('109.999'))
        assert_equal(self.nodes[2].getbalance("*"), Decimal('139.998'))
        assert_equal(self.nodes[0].getbalance("*"), Decimal('109.999'))

        # Sendmany 10 BTC with subtract fee from amount
        self.nodes[2].sendmany("", {address: 10}, 0, "", [address])
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()
        # 129.998 = 139.998 - 10
        assert_equal(self.nodes[2].getbalance(), Decimal('129.998'))
        assert_equal(self.nodes[0].getbalance(), Decimal('119.998'))
        assert_equal(self.nodes[2].getbalance("*"), Decimal('129.998'))
        assert_equal(self.nodes[0].getbalance("*"), Decimal('119.998'))

        # Test ResendWalletTransactions:
        # Create a couple of transactions, then start up a fourth
        # node (nodes[3]) and ask nodes[0] to rebroadcast.
        # EXPECT: nodes[3] should have those transactions in its mempool.
        txid1 = self.nodes[0].sendtoaddress(self.nodes[1].getnewaddress(), 1)
        txid2 = self.nodes[1].sendtoaddress(self.nodes[0].getnewaddress(), 1)
        sync_mempools(self.nodes)

        self.nodes.append(start_node(3, self.options.tmpdir))
        connect_nodes_bi(self.nodes, 0, 3)
        sync_blocks(self.nodes)

        relayed = self.nodes[0].resendwallettransactions()
        assert_equal(set(relayed), set([txid1, txid2]))
        sync_mempools(self.nodes)

        assert(txid1 in self.nodes[3].getrawmempool())

        #check if we can list zero value tx as available coins
        #1. create rawtx
        #2. hex-changed one output to 0.0
        #3. sign and send
        #4. check if recipient (node0) can list the zero value tx
        usp = self.nodes[1].listunspent()
        inputs = [{"txid":usp[0]['txid'], "vout":usp[0]['vout']}]
        outputs = {self.nodes[1].getnewaddress(): 50.00, self.nodes[0].getnewaddress(): 11.11}

        rawTx = self.nodes[1].createrawtransaction(inputs, outputs).replace("c0833842", "00000000") #replace 11.11 with 0.0 (int32)
        decRawTx = self.nodes[1].decoderawtransaction(rawTx)
        signedRawTx = self.nodes[1].signrawtransaction(rawTx)
        decRawTx = self.nodes[1].decoderawtransaction(signedRawTx['hex'])
        zeroValueTxid= decRawTx['txid']
        self.nodes[1].sendrawtransaction(signedRawTx['hex'])

        self.sync_all()
        self.nodes[1].generate(1) #mine a block
        self.sync_all()

        unspentTxs = self.nodes[0].listunspent() #zero value tx must be in listunspents output
        found = False
        for uTx in unspentTxs:
            if uTx['txid'] == zeroValueTxid:
                found = True
                assert_equal(uTx['amount'], Decimal('0.00000000'));
        assert(found)

        #do some -walletbroadcast tests
        stop_nodes(self.nodes)
        wait_bitcoinds()
        self.nodes = start_nodes(3, self.options.tmpdir, [["-walletbroadcast=0"],["-walletbroadcast=0"],["-walletbroadcast=0"]])
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        self.sync_all()

        txIdNotBroadcasted  = self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 2);
        txObjNotBroadcasted = self.nodes[0].gettransaction(txIdNotBroadcasted)
        self.sync_all()
        self.nodes[1].generate(1) #mine a block, tx should not be in there
        self.sync_all()
        assert_equal(self.nodes[2].getbalance(), Decimal('129.998')); #should not be changed because tx was not broadcasted
        assert_equal(self.nodes[2].getbalance("*"), Decimal('129.998')); #should not be changed because tx was not broadcasted

        #now broadcast from another node, mine a block, sync, and check the balance
        self.nodes[1].sendrawtransaction(txObjNotBroadcasted['hex'])
        self.sync_all()
        self.nodes[1].generate(1)
        self.sync_all()
        txObjNotBroadcasted = self.nodes[0].gettransaction(txIdNotBroadcasted)
        assert_equal(self.nodes[2].getbalance(), Decimal('131.998')); #should not be
        assert_equal(self.nodes[2].getbalance("*"), Decimal('131.998')); #should not be

        #create another tx
        txIdNotBroadcasted  = self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), 2);

        #restart the nodes with -walletbroadcast=1
        stop_nodes(self.nodes)
        wait_bitcoinds()
        self.nodes = start_nodes(3, self.options.tmpdir)
        connect_nodes_bi(self.nodes,0,1)
        connect_nodes_bi(self.nodes,1,2)
        connect_nodes_bi(self.nodes,0,2)
        sync_blocks(self.nodes)

        self.nodes[0].generate(1)
        sync_blocks(self.nodes)

        #tx should be added to balance because after restarting the nodes tx should be broadcastet
        assert_equal(self.nodes[2].getbalance(), Decimal('133.998')); #should not be
        assert_equal(self.nodes[2].getbalance("*"), Decimal('133.998')); #should not be

        # send from node 0 to node 2 taddr
        mytaddr = self.nodes[2].getnewaddress();
        mytxid = self.nodes[0].sendtoaddress(mytaddr, 10.0);
        self.sync_all()
        self.nodes[0].generate(1)
        self.sync_all()

        mybalance = self.nodes[2].z_getbalance(mytaddr)
        assert_equal(mybalance, Decimal('10.0'));

        mytxdetails = self.nodes[2].gettransaction(mytxid)
        myvjoinsplits = mytxdetails["vjoinsplit"]
        assert_equal(0, len(myvjoinsplits))

        # z_sendmany is expected to fail if tx size breaks limit
        myzaddr = self.nodes[0].z_getnewaddress()

        recipients = []
        num_t_recipients = 3000
        amount_per_recipient = Decimal('0.00000001')
        errorString = ''
        for i in xrange(0,num_t_recipients):
            newtaddr = self.nodes[2].getnewaddress()
            recipients.append({"address":newtaddr, "amount":amount_per_recipient})

        # Issue #2759 Workaround START
        # HTTP connection to node 0 may fall into a state, during the few minutes it takes to process
        # loop above to create new addresses, that when z_sendmany is called with a large amount of
        # rpc data in recipients, the connection fails with a 'broken pipe' error.  Making a RPC call
        # to node 0 before calling z_sendmany appears to fix this issue, perhaps putting the HTTP
        # connection into a good state to handle a large amount of data in recipients.
        self.nodes[0].getinfo()
        # Issue #2759 Workaround END

        try:
            self.nodes[0].z_sendmany(myzaddr, recipients)
        except JSONRPCException,e:
            errorString = e.error['message']
        assert("Too many outputs, size of raw transaction" in errorString)

        recipients = []
        num_t_recipients = 2000
        num_z_recipients = 50
        amount_per_recipient = Decimal('0.00000001')
        errorString = ''
        for i in xrange(0,num_t_recipients):
            newtaddr = self.nodes[2].getnewaddress()
            recipients.append({"address":newtaddr, "amount":amount_per_recipient})
        for i in xrange(0,num_z_recipients):
            newzaddr = self.nodes[2].z_getnewaddress()
            recipients.append({"address":newzaddr, "amount":amount_per_recipient})

        # Issue #2759 Workaround START
        self.nodes[0].getinfo()
        # Issue #2759 Workaround END

        try:
            self.nodes[0].z_sendmany(myzaddr, recipients)
        except JSONRPCException,e:
            errorString = e.error['message']
        assert("size of raw transaction would be larger than limit" in errorString)

        recipients = []
        num_z_recipients = 100
        amount_per_recipient = Decimal('0.00000001')
        errorString = ''
        for i in xrange(0,num_z_recipients):
            newzaddr = self.nodes[2].z_getnewaddress()
            recipients.append({"address":newzaddr, "amount":amount_per_recipient})
        try:
            self.nodes[0].z_sendmany(myzaddr, recipients)
        except JSONRPCException,e:
            errorString = e.error['message']
        assert("Invalid parameter, too many zaddr outputs" in errorString)

        # add zaddr to node 2
        myzaddr = self.nodes[2].z_getnewaddress()

        # send node 2 taddr to zaddr
        recipients = []
        recipients.append({"address":myzaddr, "amount":7})
        myopid = self.nodes[2].z_sendmany(mytaddr, recipients)

        opids = []
        opids.append(myopid)

        timeout = 300
        status = None
        for x in xrange(1, timeout):
            results = self.nodes[2].z_getoperationresult(opids)
            if len(results)==0:
                time.sleep(1)
            else:
                status = results[0]["status"]
                mytxid = results[0]["result"]["txid"]
                break

        assert_equal("success", status)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()

        # check balances
        zsendmanynotevalue = Decimal('7.0')
        zsendmanyfee = Decimal('0.0001')
        node2utxobalance = Decimal('143.998') - zsendmanynotevalue - zsendmanyfee

        assert_equal(self.nodes[2].getbalance(), node2utxobalance)
        assert_equal(self.nodes[2].getbalance("*"), node2utxobalance)

        # check zaddr balance
        assert_equal(self.nodes[2].z_getbalance(myzaddr), zsendmanynotevalue);

        # check via z_gettotalbalance
        resp = self.nodes[2].z_gettotalbalance()
        assert_equal(Decimal(resp["transparent"]), node2utxobalance)
        assert_equal(Decimal(resp["private"]), zsendmanynotevalue)
        assert_equal(Decimal(resp["total"]), node2utxobalance + zsendmanynotevalue)

        # there should be at least one joinsplit
        mytxdetails = self.nodes[2].gettransaction(mytxid)
        myvjoinsplits = mytxdetails["vjoinsplit"]
        assert_greater_than(len(myvjoinsplits), 0)

        # the first (probably only) joinsplit should take in all the public value
        myjoinsplit = self.nodes[2].getrawtransaction(mytxid, 1)["vjoinsplit"][0]
        assert_equal(myjoinsplit["vpub_old"], zsendmanynotevalue)
        assert_equal(myjoinsplit["vpub_new"], 0)
        assert("onetimePubKey" in myjoinsplit.keys())
        assert("randomSeed" in myjoinsplit.keys())
        assert("ciphertexts" in myjoinsplit.keys())


        # send from private note to node 0 and node 2
        node0balance = self.nodes[0].getbalance() # 28.87298817
        node2balance = self.nodes[2].getbalance() # 18.6229

        recipients = []
        recipients.append({"address":self.nodes[0].getnewaddress(), "amount":1})
        recipients.append({"address":self.nodes[2].getnewaddress(), "amount":1.0})
        myopid = self.nodes[2].z_sendmany(myzaddr, recipients)

        status = None
        opids = []
        opids.append(myopid)
        for x in xrange(1, timeout):
            results = self.nodes[2].z_getoperationresult(opids)
            if len(results)==0:
                time.sleep(1)
            else:
                status = results[0]["status"]
                break

        assert_equal("success", status)
        self.sync_all()
        self.nodes[2].generate(1)
        self.sync_all()

        node0balance += Decimal('1.0')
        node2balance += Decimal('1.0')
        assert_equal(Decimal(self.nodes[0].getbalance()), node0balance)
        assert_equal(Decimal(self.nodes[0].getbalance("*")), node0balance)
        assert_equal(Decimal(self.nodes[2].getbalance()), node2balance)
        assert_equal(Decimal(self.nodes[2].getbalance("*")), node2balance)

        #send a tx with value in a string (PR#6380 +)
        txId  = self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), "2")
        txObj = self.nodes[0].gettransaction(txId)
        assert_equal(txObj['amount'], Decimal('-2.00000000'))

        txId  = self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), "0.0001")
        txObj = self.nodes[0].gettransaction(txId)
        assert_equal(txObj['amount'], Decimal('-0.00010000'))

        #check if JSON parser can handle scientific notation in strings
        txId  = self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), "1e-4")
        txObj = self.nodes[0].gettransaction(txId)
        assert_equal(txObj['amount'], Decimal('-0.00010000'))

        #this should fail
        errorString = ""
        try:
            txId  = self.nodes[0].sendtoaddress(self.nodes[2].getnewaddress(), "1f-4")
        except JSONRPCException,e:
            errorString = e.error['message']

        assert_equal("Invalid amount" in errorString, True);

        errorString = ""
        try:
            self.nodes[0].generate("2") #use a string to as block amount parameter must fail because it's not interpreted as amount
        except JSONRPCException,e:
            errorString = e.error['message']

        assert_equal("not an integer" in errorString, True);


if __name__ == '__main__':
    WalletTest ().main ()
