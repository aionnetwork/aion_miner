# Introduction

This repo contains reference CPU and CUDA miners for the AION testnet. The reference miners have been designed to be with in conjunction with the provided reference mining pool, future versions will be adapted to join public pools. 

# Build instructions:

### Dependencies:
  - Boost 1.62+

## Windows:

Windows builds are not currently supported.

## Linux
Work in progress.
Working solvers CPU_TROMP, CUDA_TROMP

### General instructions:
  - Install **CUDA SDK v9** (make sure you have cuda libraries in **LD_LIBRARY_PATH** and cuda toolkit bins in **PATH**)
    - example on Ubuntu:
    - LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/cuda-9.0/lib64:/usr/local/cuda-9.0/lib64/stubs"
    - PATH="$PATH:/usr/local/cuda-9.0/"
    - PATH="$PATH:/usr/local/cuda-9.0/bin"

  - Use Boost 1.62+ (if it is not available from the repos you will have to download and build it yourself)
  - CMake v3.5 (if it is not available from the repos you will have to download and build it yourself)
  - Currently support only static building (CUDA_TROMP are enabled by default, check **CMakeLists.txt** in **aion_miner** root folder)

  - After that open the terminal and run the following commands:
    - `git clone https://github.com/aionnetwork/aion_miner.git`
    - `cd aion_miner`
    - `mkdir build && cd build`
    - `cmake ../MiningClient`
    - `make -j $(nproc)`

### Switching Solvers

- Navigate to the root of the project directory
- Open the CMakelists.txt file
- Toggle ON/OFF settings on lines 8 and 9 to enable/disable solvers. Rebuild project after each toggle.
- Only a **single** solver should be used at a time.

# Run instructions:

Parameters: 
	-h		Print this help and quit
	-l [location]	Stratum server:port
	-u [username]	Username (aion address)
	-a [port]	Local API port (default: 0 = do not bind)
	-d [level]	Debug print level (0 = print all, 5 = fatal only, default: 2)
	-b [hashes]	Run in benchmark mode (default: 200 iterations)

CPU settings
	-t [num_thrds]	Number of CPU threads
	-e [ext]	Force CPU ext (0 = SSE2, 1 = AVX, 2 = AVX2)

NVIDIA CUDA settings
	-ci		CUDA info
	-cd [devices]	Enable CUDA mining on spec. devices
	-cb [blocks]	Number of blocks
	-ct [tpb]	Number of threads per block
Example: -cd 0 2 -cb 12 16 -ct 64 128

If run without parameters, miner will start mining with 75% of available logical CPU cores. Use parameter -h to learn about available parameters:

## Run Examples

### CPU Example

Run AION CPU miner with 4 threads connecting to a mining pool running locally, listening on port 3333 for incoming connections.

./aionminer -t 4 -l 127.0.0.1:3333

Example to run benchmark on your CPU:

        aionminer -b

### GPU Example

Run AION CUDA miner with 64 blocks, 64 threads per block on device 0 using solver version 1 (CUDA Tromp)

./aionminer -cd 0 -cv 1 -cb 64 -ct 64 

Example to run benchmark on your GPU:

        aionminer -cd 0 -cv 1 -cb 64 -ct 64 -b



        
