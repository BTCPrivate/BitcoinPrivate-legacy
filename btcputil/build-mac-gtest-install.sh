#!/usr/bin/env bash

# Current directory
__THIS_DIR=$(pwd)


# Downloads the 1.8.0 to disc
function dl {
    printf "\n  Downloading Google Test Archive\n\n"
    curl -LO https://github.com/google/googletest/archive/release-1.8.0.tar.gz
    tar xf release-1.8.0.tar.gz
}

# Unpack and Build
function build {
    printf "\n  Building GTest and Gmock\n\n"
    cd googletest-release-1.8.0
    mkdir build 
    cd $_
    cmake -Dgtest_build_samples=OFF -Dgtest_build_tests=OFF ../
    make
}

# Install header files and library
function install {
    printf "\n  Installing GTest and Gmock\n\n"

    USR_LOCAL_INC="/usr/local/include"
    GTEST_DIR="/usr/local/Cellar/gtest/"
    GMOCK_DIR="/usr/local/Cellar/gmock/"

    mkdir $GTEST_DIR

    cp googlemock/gtest/*.a $GTEST_DIR
    cp -r ../googletest/include/gtest/  $GTEST_DIR
    ln -snf $GTEST_DIR $USR_LOCAL_INC/gtest
    ln -snf $USR_LOCAL_INC/gtest/libgtest.a /usr/local/lib/libgtest.a
    ln -snf $USR_LOCAL_INC/gtest/libgtest_main.a /usr/local/lib/libgtest_main.a

    mkdir $GMOCK_DIR
    cp googlemock/*.a   $GMOCK_DIR
    cp -r ../googlemock/include/gmock/  $GMOCK_DIR
    ln -snf $GMOCK_DIR $USR_LOCAL_INC/gmock
    ln -snf $USR_LOCAL_INC/gmock/libgmock.a /usr/local/lib/libgmock.a
    ln -snf $USR_LOCAL_INC/gmock/libgmock_main.a /usr/local/lib/libgmock_main.a
}

# Final Clean up.
function cleanup {
    printf "\n  Running Cleanup\n\n"

    cd $__THIS_DIR
    rm -rf $(pwd)/googletest-release-1.8.0
    unlink $(pwd)/release-1.8.0.tar.gz
}

dl && build && install && cleanup 
