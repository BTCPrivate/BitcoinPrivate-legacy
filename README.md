Bitcoin Private
----------------

**Bitcoin Private v1.0.10-1**

P2P Port: 7933  
RPC Port: 7932

Bitcoin Private is a fork of Zclassic, merging in the UTXO set of Bitcoin. BTCP is financial freedom.

### Info

The snapshot will take place on February 28th. The fork (creation of BTCP) will occur shortly after, on March 2nd.

Install
-----------------
### Linux

Get dependencies:
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake
```

Install:
```{r, engine='bash'}
# Build
./btcputil/build.sh -j$(nproc)
# Fetch Zcash ceremony keys
./btcputil/fetch-params.sh
# Run
./src/btcpd
```

### Windows
Get dependencies:
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake mingw-w64
```

Install (Cross-Compiled, building on Windows is not supported yet):
```{r, engine='bash'}
# Build
./btcputil/build-win.sh -j$(nproc)
```
The exe will save to `src` which you can then move to a windows machine

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


Participation in the Bitcoin Private project is subject to a
[Code of Conduct](code_of_conduct.md).

Building
--------

Build BTCP along with most dependencies from source by running
`./btcputil/build.sh`. Currently only Linux is officially supported.

License
-------

For license information see the file [COPYING](COPYING).
