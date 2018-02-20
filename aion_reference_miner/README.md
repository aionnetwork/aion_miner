# Introduction

This repo contains reference CPU and CUDA miners for the AION testnet. The reference miners have been specifically designed to be used in conjunction with the provided reference mining pool, future versions will be adapted to join public pools. 

**Note** The precompiled versions of the CPU and GPU miners have been compiled with AVX CPU features enabled. AVX features may not be supported on all CPU models, [this](#modifying-compiler-features) section outlines details to enable and disable AVX features during the build process. In order to use non-AVX versions of the miners, the miners must be built from source. 

# System Requirements
  - Ubuntu 16.04+
  - CUDA 9.0 compatible driver (NVIDIA 387.34+) for GPU based mining. 

# Quick start guide

  - Download a pre-built miner binary from the aion_miner ([release](https://github.com/aionnetwork/aion_miner/releases)) page.
  - Unpack the miner.
    ```
    tar xvjf aionminer-<TYPE>-<VERSION>.bz2
    ```
  - **Note:** When using the pre-built miner the CPU executable will be `aionminer-cpu` while the CUDA executable will be `aionminer`. When building from source the executable will be called `aionminer` for both CPU and CUDA builds. 

# Build instructions

### Dependencies
  - python-dev 2.7.11+
  - Boost 1.62+
  - cmake

## Windows

Windows builds are not currently supported.

## Linux
Work in progress.
Working solvers CPU_TROMP, CUDA_TROMP

### General instructions
  - Install Boost 1.66 example
    ```bash
    wget https://dl.bintray.com/boostorg/release/1.66.0/source/boost_1_66_0.tar.bz2
    tar --bzip2 -xf boost_1_66_0.tar.bz2
    cd boost_1_66_0
    ./bootstrap.sh
    sudo ./b2 install
    ```
  - Install **CUDA SDK v9** (make sure you have cuda libraries in **LD_LIBRARY_PATH** and cuda toolkit bins in **PATH**)
    - example on Ubuntu:
    ```bash
    LD_LIBRARY_PATH="$LD_LIBRARY_PATH:/usr/local/cuda-9.0/lib64:/usr/local/cuda-9.0/lib64/stubs"
    PATH="$PATH:/usr/local/cuda-9.0/"
    PATH="$PATH:/usr/local/cuda-9.0/bin"
    ```

  - Use Boost 1.62+ (if it is not available from the repos you will have to download and build it yourself)
  - CMake v3.5 (if it is not available from the repos you will have to download and build it yourself)
  - Currently support only static building (CUDA_TROMP are enabled by default, check **CMakeLists.txt** in **aion_miner** root folder)

  - Before proceeding ensure the correct solver is enabled
    - By default the **CUDA-Tromp** is enabled however if CUDA is not avalible this must be switch to CPU-Tromp before proceeding. The [Switch Solvers](#switching-solvers) outlines how to switch between solver implementations. 

  - To compile the miner; open the terminal and run the following commands:
    ```bash
    git clone https://github.com/aionnetwork/aion_miner.git
    cd aion_miner
    mkdir build && cd build
    cmake ../aion_reference_miner
    make
    ```
### Switching Solvers

- Navigate to the root of the project directory
- Open the CMakelists.txt file
- Toggle ON/OFF settings on lines 7 and 8 to enable/disable solvers. Rebuild project after each toggle.
- Only a **single** solver should be used at a time.

### Modifying Compiler Features

- Navigate to the root of the project directory
- Open the CMakelists.txt file
- Navigate to the Linux section at line 21.
- To disable AVX features:
  - Comment lines 26 and 27 by places a # in from of the lines.
  - Uncomment lines 23 and 24 by removing # at the start of the lines. 
- To enable AVX features:
  - Uncomment lines 26 and 27 by places a # in from of the lines.
  - Comment lines 23 and 24 by places a # in from of the lines. 

# Run instructions

Parameters: 

	-h Print this help message and quits

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

## Run Examples

### CPU Example

Run AION CPU miner with 4 threads connecting to a mining pool running locally, listening on port 3333 for incoming connections.

```./aionminer -t 4 -l 127.0.0.1:3333```

Example to run benchmark on your CPU (Single thread):

```./aionminer -b -t 1```

### GPU Example

Run AION CUDA miner with 64 blocks, 64 threads per block on device 0 using solver version 1 (CUDA Tromp)

```./aionminer -cd 0 -cv 1 -cb 64 -ct 64```

Example to run benchmark on your GPU (GPU 0, CUDA-Tromp solver):

```./aionminer -cd 0 -cv 1 -cb 64 -ct 64 -b```



        

