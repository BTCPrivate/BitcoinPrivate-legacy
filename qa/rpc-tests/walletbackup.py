#!/usr/bin/env python2
# Copyright (c) 2014 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

"""
Exercise the wallet backup code.  Ported from walletbackup.sh.

Test case is:
4 nodes. 1 2 and 3 send transactions between each other,
fourth node is a miner.
1 2 3 each mine a block to start, then
Miner creates 100 blocks so 1 2 3 each have 40 mature
coins to spend.
Then 5 iterations of 1/2/3 sending coins amongst
themselves to get transactions in the wallets,
and the miner mining one block.

Wallets are backed up using dumpwallet/backupwallet.
Then 5 more iterations of transactions and mining a block.

Miner then generates 101 more blocks, so any
transaction fees paid mature.

Sanity check:
  Sum(1,2,3,4 balances) == 114*40

1/2/3 are shutdown, and their wallets erased.
Then restore using wallet.dat backup. And
confirm 1/2/3/4 balances are same as before.

Shutdown again, restore using importwallet,
and confirm again balances are correct.
"""

from test_framework.test_framework import BitcoinTestFramework
from test_framework.authproxy import JSONRPCException
from test_framework.util import assert_equal, initialize_chain_clean, \
    start_nodes, start_node, connect_nodes, stop_node, \
    sync_blocks, sync_mempools

import os
import shutil
from random import randint
from decimal import Decimal
import logging

logging.basicConfig(format='%(levelname)s:%(message)s', level=logging.INFO)

class WalletBackupTest(BitcoinTestFramework):

    def setup_chain(self):
        logging.info("Initializing test directory "+self.options.tmpdir)
        initialize_chain_clean(self.options.tmpdir, 4)

    # This mirrors how the network was setup in the bash test
    def setup_network(self, split=False):
        # -exportdir option means we must provide a valid path to the destination folder for wallet backups
        ed0 = "-exportdir=" + self.options.tmpdir + "/node0"
        ed1 = "-exportdir=" + self.options.tmpdir + "/node1"
        ed2 = "-exportdir=" + self.options.tmpdir + "/node2"

        # nodes 1, 2,3 are spenders, let's give them a keypool=100
        extra_args = [["-keypool=100", ed0], ["-keypool=100", ed1], ["-keypool=100", ed2], []]
        self.nodes = start_nodes(4, self.options.tmpdir, extra_args)
        connect_nodes(self.nodes[0], 3)
        connect_nodes(self.nodes[1], 3)
        connect_nodes(self.nodes[2], 3)
        connect_nodes(self.nodes[2], 0)
        self.is_network_split=False
        self.sync_all()

    def one_send(self, from_node, to_address):
        if (randint(1,2) == 1):
            amount = Decimal(randint(1,10)) / Decimal(10)
            self.nodes[from_node].sendtoaddress(to_address, amount)

    def do_one_round(self):
        a0 = self.nodes[0].getnewaddress()
        a1 = self.nodes[1].getnewaddress()
        a2 = self.nodes[2].getnewaddress()

        self.one_send(0, a1)
        self.one_send(0, a2)
        self.one_send(1, a0)
        self.one_send(1, a2)
        self.one_send(2, a0)
        self.one_send(2, a1)

        # Have the miner (node3) mine a block.
        # Must sync mempools before mining.
        sync_mempools(self.nodes)
        self.nodes[3].generate(1)

    # As above, this mirrors the original bash test.
    def start_three(self):
        self.nodes[0] = start_node(0, self.options.tmpdir)
        self.nodes[1] = start_node(1, self.options.tmpdir)
        self.nodes[2] = start_node(2, self.options.tmpdir)
        connect_nodes(self.nodes[0], 3)
        connect_nodes(self.nodes[1], 3)
        connect_nodes(self.nodes[2], 3)
        connect_nodes(self.nodes[2], 0)

    def stop_three(self):
        stop_node(self.nodes[0], 0)
        stop_node(self.nodes[1], 1)
        stop_node(self.nodes[2], 2)

    def erase_three(self):
        os.remove(self.options.tmpdir + "/node0/regtest/wallet.dat")
        os.remove(self.options.tmpdir + "/node1/regtest/wallet.dat")
        os.remove(self.options.tmpdir + "/node2/regtest/wallet.dat")

    def run_test(self):
        logging.info("Generating initial blockchain")
        self.nodes[0].generate(1)
        sync_blocks(self.nodes)
        self.nodes[1].generate(1)
        sync_blocks(self.nodes)
        self.nodes[2].generate(1)
        sync_blocks(self.nodes)
        self.nodes[3].generate(100)
        sync_blocks(self.nodes)

        assert_equal(self.nodes[0].getbalance(), 50.00)
        assert_equal(self.nodes[1].getbalance(), 50.00)
        assert_equal(self.nodes[2].getbalance(), 50.00)
        assert_equal(self.nodes[3].getbalance(), 0)

        logging.info("Creating transactions")
        # Five rounds of sending each other transactions.
        for i in range(5):
            self.do_one_round()

        logging.info("Backing up")
        tmpdir = self.options.tmpdir
        self.nodes[0].backupwallet("walletbak")
        self.nodes[0].dumpwallet("walletdump")
        self.nodes[1].backupwallet("walletbak")
        self.nodes[1].dumpwallet("walletdump")
        self.nodes[2].backupwallet("walletbak")
        self.nodes[2].dumpwallet("walletdump")

        # Verify dumpwallet cannot overwrite an existing file
        try:
            self.nodes[2].dumpwallet("walletdump")
            assert(False)
        except JSONRPCException as e:
            errorString = e.error['message']
            assert("Cannot overwrite existing file" in errorString)

        logging.info("More transactions")
        for i in range(5):
            self.do_one_round()

        # Generate 101 more blocks, so any fees paid mature
        self.nodes[3].generate(101)
        self.sync_all()

        balance0 = self.nodes[0].getbalance()
        balance1 = self.nodes[1].getbalance()
        balance2 = self.nodes[2].getbalance()
        balance3 = self.nodes[3].getbalance()
        total = balance0 + balance1 + balance2 + balance3

        # At this point, there are 214 blocks (103 for setup, then 10 rounds, then 101.)
        # 114 are mature, so the sum of all wallets should be 100*11.4375 + 4 * 11 + 10*8.75 = 1275.25
        assert_equal(total, 5700)

        ##
        # Test restoring spender wallets from backups
        ##
        logging.info("Restoring using wallet.dat")
        self.stop_three()
        self.erase_three()

        # Start node2 with no chain
        shutil.rmtree(self.options.tmpdir + "/node2/regtest/blocks")
        shutil.rmtree(self.options.tmpdir + "/node2/regtest/chainstate")

        # Restore wallets from backup
        shutil.copyfile(tmpdir + "/node0/walletbak", tmpdir + "/node0/regtest/wallet.dat")
        shutil.copyfile(tmpdir + "/node1/walletbak", tmpdir + "/node1/regtest/wallet.dat")
        shutil.copyfile(tmpdir + "/node2/walletbak", tmpdir + "/node2/regtest/wallet.dat")

        logging.info("Re-starting nodes")
        self.start_three()
        sync_blocks(self.nodes)

        assert_equal(self.nodes[0].getbalance(), balance0)
        assert_equal(self.nodes[1].getbalance(), balance1)
        assert_equal(self.nodes[2].getbalance(), balance2)

        logging.info("Restoring using dumped wallet")
        self.stop_three()
        self.erase_three()

        #start node2 with no chain
        shutil.rmtree(self.options.tmpdir + "/node2/regtest/blocks")
        shutil.rmtree(self.options.tmpdir + "/node2/regtest/chainstate")

        self.start_three()

        assert_equal(self.nodes[0].getbalance(), 0)
        assert_equal(self.nodes[1].getbalance(), 0)
        assert_equal(self.nodes[2].getbalance(), 0)

        self.nodes[0].importwallet(tmpdir + "/node0/walletdump")
        self.nodes[1].importwallet(tmpdir + "/node1/walletdump")
        self.nodes[2].importwallet(tmpdir + "/node2/walletdump")

        sync_blocks(self.nodes)

        assert_equal(self.nodes[0].getbalance(), balance0)
        assert_equal(self.nodes[1].getbalance(), balance1)
        assert_equal(self.nodes[2].getbalance(), balance2)


if __name__ == '__main__':
    WalletBackupTest().main()
