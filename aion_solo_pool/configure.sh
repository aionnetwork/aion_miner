#!/bin/bash
# Configure all settings for aion_solo_pool

REDIS_DOWNLOAD="http://download.redis.io/releases/redis-4.0.8.tar.gz"
NODE_DOWNLOAD="https://nodejs.org/dist/v8.9.4/node-v8.9.4-linux-x64.tar.xz"

#clean
rm -rf ./redis.tar.gz ./node.tar.gz ./redis ./node &&

echo -e "\e[32mFetching dependencies."
tput sgr0
#download
wget $REDIS_DOWNLOAD -O ./redis.tar.gz &&
wget $NODE_DOWNLOAD -O ./node.tar.gz &&
tar -xf ./redis.tar.gz &&
mv ./redis-4.0.8 ./redis &&
tar -xf ./node.tar.gz &&
mv ./node-v8.9.4-linux-x64 ./node &&


# Build Redis
echo -e "\e[32mBuilding Redis."
tput sgr0
make -C ./redis

# Build NPM 
echo -e "\e[32mInstalling NPM modules."
tput sgr0
export PATH=$PATH:$PWD/node/bin
npm install

# Clean
rm -r ./redis.tar.gz ./node.tar.gz
