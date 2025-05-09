#!/bin/bash

sudo apt install cmake g++ clang-format clang-tidy python3-pip pkg-config curl zip unzip tar -y


# Installing vcpkg
wget -O vcpkg.tar.gz https://github.com/microsoft/vcpkg/archive/refs/tags/2025.02.14.tar.gz
mkdir ~/vcpkg
tar xf vcpkg.tar.gz --strip-components=1 -C ~/vcpkg
~/vcpkg/bootstrap-vcpkg.sh
sudo ln -s ~/vcpkg/vcpkg /usr/local/bin/vcpkg
rm -rf vcpkg.tar.gz


# Installing android-sdk ndk
sudo apt install google-android-platform-34-installer -y
sudo apt install google-android-build-tools-34.0.0-installer -y
sudo apt install google-android-cmdline-tools-13.0-installer -y
sudo apt install google-android-ndk-r25-installer -y
