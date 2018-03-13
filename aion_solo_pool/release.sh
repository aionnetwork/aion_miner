#REDIS_DOWNLOAD="http://download.redis.io/releases/redis-4.0.8.tar.gz"
#NODE_DOWNLOAD="https://nodejs.org/dist/v8.9.4/node-v8.9.4-linux-x64.tar.xz"

#clean
#rm -rf ./redis.tar.gz ./node.tar.gz ./redis ./node &&

#download
#wget $REDIS_DOWNLOAD -O ./redis.tar.gz &&
#wget $NODE_DOWNLOAD -O ./node.tar.gz &&
#tar -xf ./redis.tar.gz &&
#mv ./redis-4.0.8 ./redis &&
#tar -xf ./node.tar.gz &&
#mv ./node-v8.9.4-linux-x64 ./node &&

#pack
node node_modules/gulp/bin/gulp.js release 

#clean
#rm -r ./redis.tar.gz ./node.tar.gz ./redis ./node
