#!/usr/bin/env python2
# Copyright (c) 2014 The Bitcoin Core developers
# Distributed under the MIT software license, see the accompanying
# file COPYING or http://www.opensource.org/licenses/mit-license.php.

#
# Test node handling
#

from test_framework.test_framework import BitcoinTestFramework
from test_framework.util import assert_equal, connect_nodes_bi, p2p_port,connect_nodes

import time

try:
    import urllib.parse as urlparse
except ImportError:
    import urlparse

class NodeHandlingTest (BitcoinTestFramework):
    def run_test(self):
        ###########################
        # setban/listbanned tests #
        ###########################
        assert_equal(len(self.nodes[2].getpeerinfo()), 4) #we should have 4 nodes at this point
        self.nodes[2].setban("127.0.0.1", "add")
        time.sleep(3) #wait till the nodes are disconected
        assert_equal(len(self.nodes[2].getpeerinfo()), 0) #all nodes must be disconnected at this point
        assert_equal(len(self.nodes[2].listbanned()), 1)
        self.nodes[2].clearbanned()
        assert_equal(len(self.nodes[2].listbanned()), 0)
        self.nodes[2].setban("127.0.0.0/24", "add")
        assert_equal(len(self.nodes[2].listbanned()), 1)
        try:
            self.nodes[2].setban("127.0.0.1", "add") #throws exception because 127.0.0.1 is within range 127.0.0.0/24
        except:
            pass
        assert_equal(len(self.nodes[2].listbanned()), 1) #still only one banned ip because 127.0.0.1 is within the range of 127.0.0.0/24
        try:
            self.nodes[2].setban("127.0.0.1", "remove")
        except:
            pass
        assert_equal(len(self.nodes[2].listbanned()), 1)
        self.nodes[2].setban("127.0.0.0/24", "remove")
        assert_equal(len(self.nodes[2].listbanned()), 0)
        self.nodes[2].clearbanned()
        assert_equal(len(self.nodes[2].listbanned()), 0)
        
        ###########################
        # RPC disconnectnode test #
        ###########################
        url = urlparse.urlparse(self.nodes[1].url)
        self.nodes[0].disconnectnode(url.hostname+":"+str(p2p_port(1)))
        time.sleep(2) #disconnecting a node needs a little bit of time
        for node in self.nodes[0].getpeerinfo():
            assert(node['addr'] != url.hostname+":"+str(p2p_port(1)))

        connect_nodes_bi(self.nodes,0,1) #reconnect the node
        found = False
        for node in self.nodes[0].getpeerinfo():
            if node['addr'] == url.hostname+":"+str(p2p_port(1)):
                found = True
        assert(found)

        ###########################
        # Connection to self (takes approx 5 min)
        # Stress test for network layer. Trying to connect to self every 0.5 sec.
        # Helps to discover different multi-threading problems.
        ###########################
        url = urlparse.urlparse(self.nodes[0].url)

        print "Connection to self stress test. " \
              "Constantly trying to connect to self every 0.5 sec. " \
              "The whole test takes approx 5 mins"
        for x in xrange(600):
            connect_nodes(self.nodes[0], 0)
            time.sleep(0.5)
            # self-connection should be disconnected during the version checking
            for node in self.nodes[0].getpeerinfo():
                assert(node['addr'] != url.hostname+":"+str(p2p_port(0)))

if __name__ == '__main__':
    NodeHandlingTest ().main ()
