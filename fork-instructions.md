
# Instructions to set up a BitcoinPrivate full-node to Mine the Fork
Linux ONLY, 500GB+ recommended

*You will not receive a mining reward if you choose to help mine the fork*

**Bitcoin Private**

March 2, 2018

#

**Install Bitcoin Private**

Follow the instructions [here](https://github.com/BTCPrivate/BitcoinPrivate/blob/master/README.md).

**Download + Decompress Snapshot Data (BTC UTXOs)**
```
cd ~/.btcprivate/
curl https://s3.amazonaws.com/btcp.snapshot/utxo_snapshot.tar.gz | tar xvz
```

You can also generate the utxo_snapshot yourself [here](https://github.com/BTCPrivate/utxo_dump).

**Make the config file**
```
mkdir ~/.btcprivate
touch ~/.btcprivate/btcprivate.conf
```

**Run the daemon**
```
cd ~/BitcoinPrivate
./src/btcpd
```
