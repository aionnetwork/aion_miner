#!/bin/bash

#cd ./redis && make
make -C ./redis
./redis/src/redis-server --daemonize yes