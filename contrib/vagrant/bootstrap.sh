#!/usr/bin/env bash

# this might take a while
sudo apt-get update
sudo apt-get install -y software-properties-common

# add the ppa for cmake
sudo add-apt-repository -y ppa:george-edison55/cmake-3.x
sudo apt-get update

# install dependencies
sudo apt-get install -y cmake libicu-dev git g++ g++-4.8

# clone the project
git clone https://github.com/meta-toolkit/meta.git
cd meta/

# uncomment to build exact version
# git reset --hard v1.3.2

# set up submodules
git submodule update --init --recursive

# set up a build directory
mkdir build
cd build
cp ../config.toml .

# configure and build the project
cmake ../ -DCMAKE_BUILD_TYPE=Release
make