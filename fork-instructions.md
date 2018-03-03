#
# Instructions to set up a BitcoinPrivate full-node
Linux ONLY, 500GB+ recommended
#
# Bitcoin Private
March 2, 2018
#

# Install Bitcoin Private

Follow the instructions [here](https://github.com/BTCPrivate/BitcoinPrivate/blob/master/README.md).

# Download + Decompress Snapshot Data (BTC UTXOs)
```
cd ~/.btcprivate/
curl https://s3.amazonaws.com/btcp.snapshot/utxo_snapshot.tar.gz | tar xvz
```

# Make the config file
```
mkdir ~/.btcprivate
touch ~/.btcprivate/btcprivate.conf
```

# Run the daemon
```
cd ~/BitcoinPrivate
./src/btcpd
```
