# Aion Solo Mining Pool

## About

This is an Aion mining pool designed to be used in conjunction with the Aion mining client to be used on the Aion testnet. This mining pool has been specifically designed to be used only for solo mining on the Aion test network; it is not suitable to be used as a public mining pool and should not be deployed in that configuration.

## Quick start guide 

### Requirements (quickstart)
- **Aion kernel** ([download and install](https://github.com/aionnetwork/aion))
- **Python v2.7**
  - Included by default with Ubuntu desktop, may need to be installed seperatly in Ubuntu server. 
  - To install: ```sudo apt-get update && sudo apt-get install build-essential```
- **make** (Included with Ubuntu)
  - Included by default with Ubuntu desktop, may need to be installed seperatly in Ubuntu server. 
  - To install: ```sudo apt-get install python2.7 python-dev```

### Instructions

- Open 2 terminal windows; using the first window navigate to your Aion kernel.
- Open the Aion configuration located in config/config.xml
- Disable kernel mining.
- Set the miner address to the address which will receive mined block rewards. The address is a 64 character (32 byte) hex string containing the public key and address of an account. The address MUST begin with a0 as these these are the only valid addresses in the Aion network. 

  Eg.
  ```
    <consensus>
            <mining>false</mining>
            <miner-address>a0b91f3053ee1b87ac5a7a1d9de0f5a14b71b642ae1d872f70794970f09a5a2</miner-address>
            <cpu-mine-threads>8</cpu-mine-threads>
            <extra-data>AION</extra-data>
    </consensus>
  ```
  
- Ensure the stratum API is enabled.

Eg. 

```
   <apis-enabled>web3,eth,personal,stratum</apis-enabled>
```


- Download the latest prepackaged aion_solo_pool on the ([release](https://github.com/aionnetwork/aion_miner/releases)) page.
- Place the download into the directory from which you plan to run the pool.
- Using the 2nd terminal window, navigate to the download directory and Unpack the solo pool.
  ```
  tar xf aion-solo-pool-<VERSION>.tar.gz
  ```
- Run the configure script; this script will download and build all of the pool dependencies and place them into the current directory. This script may take several minutes to complete however it must only be run once. 
  ```
  ./configure.sh
  ```
- Run the solo_pool using the quickstart run script. This script will start and stop both the pool and redis server. 
  ```
  ./run_quickstart.sh
  ```
- Start the Aion kernel in the first terminal window. 
- The pool is now ready to accept incoming client connections and to distribute work to clients. Using default settings the pool will accept client connections on port 3333 and connect to the Aion kernel on port 8545. 

## Requirements (building from source)
* **Aion kernel** ([download and install](https://github.com/aionnetwork/aion))
* **Node.js** v8.9.3+ ([download and install](https://nodejs.org/en/download/))
* **Redis** key-value store v2.6+ ([download and install](http://redis.io/topics/quickstart))
* **Python v2.7**
* **make**
* **node-gyp** v3.6.2+ ([download and install](https://github.com/nodejs/node-gyp))

## Setup

#### 0) Setting up Aion (Optional)

The default Aion IP and port binding values are 127.0.0.1 (localhost) port 8545. These values may be changed by modify the following lines within the config/config.xml file located within the Aion kernel install folder. The IP and port bindings may be set to desired values; however this guide will assume a default binding of 127.0.0.1:8545.

```
<api>
        <rpc active="true" ip="127.0.0.1" port="8545"></rpc>
        ....
</api>
```

#### 1) Clone the repository locally

```git clone https://github.com/aionnetwork/aion_miner.git```

#### 2) Verify the pool has been correctly configured for the Aion RPC connections
- Navigate to the pool_configs folder and open aion.json.
- Scroll to the ```daemons``` section.
- Ensure the daemon configuration matches the Aion IP and port binding from the previous step. 

Eg. Using default settings the configuration should be:

```
    "daemons": [
        {
            "host": "127.0.0.1",
            "port": 8545,
            "user": "",
            "password": ""
        }
    ]
```

#### 3) Install node modules

- Navigate to the root of the pool directory.
- Run the command 
```
npm install
``` 
and allow all required npm modules to be installed in the node_modules folder.

#### 4) Verify the equihash verifier build

- Navigate to ```local_modules/equihashverify```
- Run test with the following command 
```
node test.js
```
- A successful test should output: 
```
Header length: 64
Solution length: 1408
true
```

#### 5) Start Redis Server

- Navigate to the Redis install folder.
- Start the Redis server ```(./src/redis-server)``` from the root of the redis install folder. 

#### 6) Verify Aion configuration
- Open the Aion config in the root Aion folder /config/config.xml.
- Navigate to the consensus section.
- Disable kernel mining.
- Set the miner address to the address which will receive mined block rewards. The address is a 64 character (32 byte) hex string containing the public key and address of an account. 

Eg.

  ```
    <consensus>
            <mining>false</mining>
            <miner-address>a0b91f3053ee1b87ac5a7a1d9de0f5a14b71b642ae1d872f70794970f09a5a2</miner-address>
            <cpu-mine-threads>8</cpu-mine-threads>
            <extra-data>AION</extra-data>
    </consensus>
  ```

#### 7) Start the Aion kernel

- Navigate to the aion install folder
- Start the kernel ```./aion.sh```

#### 8) Start the mining pool

- Navigate to the base mining pool folder
- Start the mining pool ```./run.sh```
- Ensure the pool starts with no error messages.

At this stage the mining pool is ready to receive client connections and to distribute work. 

#### 9) Validate client connections (Optional)

- The pool is configured to listen for client connections on port 3333 by default. This may be changed in the config.json file located in the root of the pool folder. 
- Connect one of the solo mining clients to the pool using a location of **127.0.0.1:3333**. 
- Once connected the client should begin receiving work within several seconds; if receiving work the pool has been successfully configured. 

License
-------
Released under the GNU General Public License v2

http://www.gnu.org/licenses/gpl-2.0.html
