Connecting to testnet
==========================================================

First, follow the build instructions in [README](/README.md)
To run btcpd and connect to testnet

#### Linux
```
mkdir ~/.btcprivate
touch ~/.btcprivate/btcprivate.conf
./src/btcpd -testnet -gen
```

#### Windows
Click on Start/Windows Logo and type `cmd` once there type `%AppData%\BitcoinPrivate`. It will open the explorer in that directory.Create a new text file and name it as `btcprivate.conf` (be sure you remove the txt extension) in the directory. An empty file is sufficient
Execute the btcd binary

Once you run the daemon, it will first sync the chain. After it is synced it will begin mining blocks. If you don't wish to mine remov the -gen option from the command line.
Now that you have your node up you can use `btcp-cli` to make RPC calls to your node (i.e `btcp-cli getinfo`).
