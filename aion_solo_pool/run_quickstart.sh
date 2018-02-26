#!/bin/bash

trap ctrl_c INT

function ctrl_c() {
        echo
        echo -e "\e[31mShutting down aion_solo__pool"
        echo
        kill -9 `pgrep redis-server`
}

./redis/src/redis-server --daemonize yes
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/node_modules/stratum-pool/node_modules/equihashverify/build/Release/:$PWD/node_modules/equihashverify/build/Release/ \
node init.js
