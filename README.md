Bitcoin Private v1.0.10-1

NOTICE, the default ports have changed! The p2p port is now 7933 and rpcport is 7932

What is Bitcoin Private?
----------------
Bitcoin Private is financial freedom.

Install
-----------------
### Linux

Get dependencies
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake
```

Install
```{r, engine='bash'}
# Build
./btcputil/build.sh -j$(nproc)
# fetch key
./btcputil/fetch-params.sh
# Run
./src/btcpd
```

### Windows
Get dependencies
```{r, engine='bash'}
sudo apt-get install \
      build-essential pkg-config libc6-dev m4 g++-multilib \
      autoconf libtool ncurses-dev unzip git python \
      zlib1g-dev wget bsdmainutils automake mingw-w64
```

Install (Cross-Compiled, building on Windows is not supported yet)
```{r, engine='bash'}
# Build
./btcputil/build-win.sh -j$(nproc)
```
The exe will save to `src` which you can then move to a windows machine

### Mac
Get dependencies
```{r, engine='bash'}
#install xcode
xcode-select --install

/usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"
brew install cmake autoconf libtool automake coreutils pkgconfig gmp wget

brew install gcc5 --without-multilib
```

Install
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

[Bitcoin Private](http://zclassic.org/), like [Zcash](https://z.cash/), is an implementation of the "Zerocash" protocol.
Based on Bitcoin's code, it intends to offer a far higher standard of privacy
through a sophisticated zero-knowledge proving scheme that preserves
confidentiality of transaction metadata. Technical details are available
in the Zcash [Protocol Specification](https://github.com/zcash/zips/raw/master/protocol/protocol.pdf).

This software is the Bitcoin Private client. It downloads and stores the entire history
of Bitcoin Private transactions; depending on the speed of your computer and network
connection, the synchronization process could take a day or more once the
blockchain has reached a significant size.

Security Warnings
-----------------

See important security warnings in
[doc/security-warnings.md](doc/security-warnings.md).

**Bitcoin Private and Zcash are unfinished and highly experimental.** Use at your own risk.

Deprecation Policy
------------------

This release is considered deprecated 16 weeks after the release day. There
is an automatic deprecation shutdown feature which will halt the node some
time after this 16 week time period. The automatic feature is based on block
height and can be explicitly disabled.

Where do I begin?
-----------------
We have a guide for joining the main Bitcoin Private network:
https://github.com/BTCPrivate/BitcoinPrivate/wiki/1.0-User-Guide

### Need Help?

* See the documentation at the [Bitcoin Private Wiki](https://github.com/BTCPrivate/BitcoinPrivate/wiki)
  for help and more information.
* Ask for help on the [Bitcoin Private](http://zcltalk.tech/index.php) forum.

### Want to participate in development?

* Code review is welcome!
* If you want to get to know us join our slack: http://zclassic.herokuapp.com/


Participation in the Zcash project is subject to a
[Code of Conduct](code_of_conduct.md).

Building
--------

Build Zcash along with most dependencies from source by running
`./btcputil/build.sh`. Currently only Linux is officially supported.

License
-------

For license information see the file [COPYING](COPYING).
