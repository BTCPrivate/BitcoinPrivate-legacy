
Connecting to testnet
==========================================================

After building Bitcoin private, you should have two binaries (btcpd, and btcp-cli) on your build target directory, but before running the btcp, you need to configure it:

1. Create a btcprivate.conf file

#### Linux 

```
vi ~/.btcprivate/btcprivate.conf
```

#### Windows

Click on Start/Windows Logo and type `cmd` once there type `%AppData%\BitcoinPrivate`. It will open the explorer in that directory. Create a new text file and name it as `btcprivate.conf` (be sure you remove the txt extension) in the directory and open it with a text editor

2. Add the following to the conf file

```
testnet=1
addnode=testnet1.btcprivate.org:17933

rpcuser=test
rpcpassword=test
rpcport=17932
rpcallowip=127.0.0.1

```

The rpc parameters are required for the daemon to run. the `testnet=1` tells the daemon to connect to testnet, and `addnode=...` is to connect to a reliable node. This is a temporal until we add dns seeds.

3. Execute the btcd binary

Once you run the daemon, you should first wait to sync the chain before mining. Once you are synced you can add `gen=1` to the conf file, and restart the btcp daemon.

Now that you have your node up you can use `btcp-cli` to make RPC calls to your node (i.e `btcp-cli getinfo`).
