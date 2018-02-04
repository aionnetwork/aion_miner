nohup bash ./redis.sh > /dev/null 2>&1 &
echo "running redis server" && echo $PWD &&
export PATH=$PATH:$PWD/node/bin &&
npm install &&
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$PWD/node_modules/stratum-pool/node_modules/equihashverify/build/Release/:$PWD/node_modules/equihashverify/build/Release/ \
node init.js