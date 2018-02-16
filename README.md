# Aion Miner

### About
Welcome to the Aion mining repository, this repository will contain documentation, mining details and reference implementations of CPU and CUDA miners for the Aion blockchain. 

### Algorithm Details

Aion leverages an enhanced Equihash algorithm for its Proof of Work (PoW). Details regarding changes to the algorithm may be found in the in [wiki](https://github.com/aionnetwork/aion_miner/wiki/Aion-equihash_210_9--specification-and-migration-guide.).

### Reference Miners

In addition to providing a miner within the Aion kernel, two reference miners (CPU, GPU-CUDA) and a solo mining pool have been provided. 

The solo mining pool may be found [here](https://github.com/aionnetwork/aion_miner/tree/master/aion_solo_pool).

The reference mining clients may be found [here](https://github.com/aionnetwork/aion_miner/tree/master/aion_reference_miner).

**Note** In order to use the reference mining clients the the solo_mining_pool must be configured and run in parallel with the reference minining clients. The solo mining pool serves as the connection between the mining client and the aion kernel. 

Additionally, recompiled releases for both the mining pool as well as mining clients may be found under the [releases](https://github.com/aionnetwork/aion_miner/releases) page as they become available. 
