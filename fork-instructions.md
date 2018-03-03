#
# Instructions to set up a BitcoinPrivate full-node
# Linux ONLY, 500GB+ recommended
#
# Bitcoin Private
# March 2, 2018
#

# Install Dependencies
sudo apt-get update 

sudo apt-get -y install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils make automake

# OPTIONAL: Make sure you have a big enough Swapfile
#cd /
#sudo dd if=/dev/zero of=swapfile bs=1M count=3000
#sudo mkswap swapfile
#sudo chmod 0600 /swapfile
#sudo swapon swapfile
#echo "/swapfile none swap sw 0 0" | sudo tee -a etc/fstab > /dev/null

cd ~

# Clone the BitcoinPrivate repo
git clone https://github.com/BTCPrivate/BitcoinPrivate
cd BitcoinPrivate

# Build Wallet / Daemon
./btcputil/fetch-params.sh
./btcputil/build.sh -j$(nproc)

cd ~/.btcprivate/

# Download + Decompress Snapshot Data (BTC UTXOs)
curl https://s3.amazonaws.com/btcp.snapshot/utxo_snapshot.tar.gz | tar xvz

# Run the daemon
cd ~/BitcoinPrivate
./src/btcpd
