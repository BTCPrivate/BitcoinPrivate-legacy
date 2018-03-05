Bitcoin Private
----------------

**Bitcoin Private v1.0.10-1**

P2P Port: 7933

RPC Port: 7932

Bitcoin Private is a fork of Zclassic, merging in the UTXO set of Bitcoin. BTCP is financial freedom.

### Info

The snapshot will take place on February 28th. The fork (creation of BTCP) will occur shortly after, on March 2nd.

Build
-----------------
### Linux

Get dependencies:
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake
```

Build:
```{r, engine='bash'}
# Checkout
git clone https://github.com/BTCPrivate/BitcoinPrivate.git
cd BitcoinPrivate
# Build
./btcputil/build.sh -j$(nproc)
# Fetch Zcash ceremony keys
./btcputil/fetch-params.sh
```

Create Config File:
```
mkdir ~/.btcprivate
touch ~/.btcprivate/btcprivate.conf
vi ~/.btcprivate/btcprivate.conf
```

Add following lines to `btcprivate.conf` and be sure to change the rpcpassword:
```
rpcuser=btcprivaterpc
rpcpassword=set-a-password
rpcallowip=127.0.0.1
addnode=dnsseed.btcprivate.org
```


Run:
```
./src/btcpd
```

### Windows
Windows is not an officially or fully supported build - however there are two ways to build BTCP for Windows:

* On Linux using [Mingw-w64](https://mingw-w64.org/doku.php) cross compiler tool chain. Ubuntu 16.04 Xenial is proven to work and the instructions is for such release.
* On Windows 10 (64-bit version) using [Windows Subsystem for Linux (WSL)](https://msdn.microsoft.com/commandline/wsl/about) and Mingw-w64 cross compiler tool chain.

With Windows 10, Microsoft released a feature called WSL. It basically allows you to run a bash shell directly on Windows in an ubuntu environment. WSL can be installed with other Linux variants, but as mentioned before, the distro proven to work is Ubuntu.
Follow this [link](https://msdn.microsoft.com/en-us/commandline/wsl/install_guide) for installing WSL first

### Building for Windows 64-Bit
1. Get the usual dependencies:
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake make cmake mingw-w64
```

2. Set the default ming32 gcc/g++ compiler option to posix, fix problem with packages in Xenial

```{r, engine='bash'}
sudo apt install software-properties-common
sudo add-apt-repository "deb http://archive.ubuntu.com/ubuntu zesty universe"
sudo apt update
sudo apt upgrade
sudo update-alternatives --config x86_64-w64-mingw32-gcc
sudo update-alternatives --config x86_64-w64-mingw32-g++
```

3. Install Rust
```{r, engine='bash'}
curl https://sh.rustup.rs -sSf | sh
source ~/.cargo/env
rustup install stable-x86_64-unknown-linux-gnu
rustup install stable-x86_64-pc-windows-gnu
rustup target add x86_64-pc-windows-gnu
vi  ~/.cargo/config
```
and add:
```
[target.x86_64-pc-windows-gnu]
linker = "/usr/bin/x86_64-w64-mingw32-gcc"
```

Note that in WSL, the BTCPrivate source code must be somewhere in the default mount file system. i.e /usr/src/BTCPrivate, and not on /mnt/d/. What this means is that you cannot build directly on the windows system

4. Build for Windows

```{r, engine='bash'}
PATH=$(echo "$PATH" | sed -e 's/:\/mnt.*//g') # strip out problematic Windows %PATH% imported var
./btcputil/build-win.sh -j$(nproc)
```

5. Installation

If compiling on linux, use the following command to build the executables in `./depends/x86_64-w64-mingw32/bin/

```{r, engine='bash'}
sudo make install DESTDIR=
```

If compiling using WSL, use the following command to build the executables in `c:\btcp\BTCPrivate

```{r, engine='bash'}
sudo make install DESTDIR=/mnt/c/btcp/BTCPrivate
```

### Mac
Get dependencies:
```{r, engine='bash'}
#install xcode
xcode-select --install

/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install cmake autoconf libtool automake coreutils pkgconfig gmp wget

brew install gcc5 --without-multilib
```

Install:
```{r, engine='bash'}
# Build
./btcputil/build-mac.sh -j$(sysctl -n hw.physicalcpu)
# fetch key
./btcputil/fetch-params.sh
# Run
./src/btcpd
```

### Additional notes

If you plan to build for windows and linux at the same time, be sure to delete all the built files for whatever you build first. An easy way to do this is by taking the binaries out of the repo, delete all files except the .git folder and then do a git hard reset.

### Testnet

Build the latest version of BTCP using the instructions below, then follow the [testnet guide](doc/testnet.md)

About
--------------

[Bitcoin Private](http://zclassic.org/), like [Zclassic](https://zclassic.org/) and [Zcash](https://z.cash/), is an implementation of the "Zerocash" protocol.
Based on Zclassic's code, it intends to offer a far higher standard of privacy
through a sophisticated zero-knowledge proving scheme that preserves
confidentiality of transaction metadata. Technical details are available
in the Zcash [Protocol Specification](https://github.com/zcash/zips/raw/master/protocol/protocol.pdf).

This software is the Bitcoin Private client. It downloads and stores the entire history
of Bitcoin Private transactions. Depending on the speed of your computer and network
connection, the synchronization process could take a day or more once the
blockchain has reached a significant size.

It includes both `btcpd` (the daemon) and `btcp-cli` (the command line tools).

Security Warnings
-----------------

See important security warnings in
[doc/security-warnings.md](doc/security-warnings.md).

**Bitcoin Private is unfinished and highly experimental.** Use at your own risk.

Deprecation Policy
------------------

This release is considered deprecated 16 weeks after the release day. There
is an automatic deprecation shutdown feature which will halt the node some
time after this 16 week time period. The automatic feature is based on block
height and can be explicitly disabled.

Where do I begin?
-----------------
We have a guide for joining the main Bitcoin Private network:
https://github.com/zcash/zcash/wiki/1.0-User-Guide

### Need Help?

* See documentation at the [Zcash Wiki](https://github.com/zcash/zcash/wiki) and the [Zclassic Wiki](https://github.com/z-classic/zclassic/wiki)
  for help and more information.

### Want to participate in development?

* Code review is welcome!
* If you want to get to know us join our Discord: https://discord.gg/9xezcaK
* We have a brief guide for joining the Bitcoin private testnet [here](doc/testnet.md)


Participation in the Bitcoin Private project is subject to a
[Code of Conduct](code_of_conduct.md).

Building
--------

Build BTCP along with most dependencies from source by running
`./btcputil/build.sh`. Currently only Linux is officially supported.

License
-------

For license information see the file [COPYING](COPYING).
