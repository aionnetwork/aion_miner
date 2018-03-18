const bignum = require('bignum')
const merkleTree = require('./merkleTree.js')
const transactions = require('./transactions.js')
const util = require('./util.js')
const diff1 = 0x00000000ffff0000000000000000000000000000000000000000000000000000

/**
 * The BlockTemplate class holds a single job.
 * and provides several methods to validate and submit it to the daemon coin
**/
module.exports = function BlockTemplate(
    jobId,
    rpcData,
    poolAddressScript,
    extraNoncePlaceholder,
    reward,
    txMessages,
    recipients
){
    //private members

    let submits = []

    function getMerkleHashes(steps){
        return steps.map(function(step){
            return step.toString('hex')
        })
    }

    function getTransactionBuffers(txs){
        var txHashes = txs.map(function(tx){
            if (tx.txid !== undefined) {
                return util.uint256BufferFromHash(tx.txid)
            }
            return util.uint256BufferFromHash(tx.hash)
        })
        return [null].concat(txHashes)
    }

    function getVoteData(){
        if (!rpcData.masternode_payments) return new Buffer([])

        return Buffer.concat(
            [util.varIntBuffer(rpcData.votes.length)].concat(
                rpcData.votes.map(function (vt) {
                    return new Buffer(vt, 'hex')
                })
            )
        )
    }

    //public members

    this.rpcData = rpcData
    this.jobId = jobId
    this.headerHash = this.rpcData.headerHash

    this.target = rpcData.target ?
        bignum(rpcData.target, 16) :
        util.bignumFromBitsHex(rpcData.bits)

    this.difficulty = parseFloat((diff1 / this.target.toNumber()).toFixed(9))

    this.prevHashReversed = util.reverseByteOrder(new Buffer(rpcData.previousblockhash, 'hex')).toString('hex')
    this.transactionData = Buffer.concat(rpcData.transactions.map(function(tx){
        return new Buffer(tx.data, 'hex')
    }))
    this.merkleTree = new merkleTree(getTransactionBuffers(rpcData.transactions))
    this.merkleBranch = getMerkleHashes(this.merkleTree.steps)
    this.generationTransaction = transactions.CreateGeneration(
        rpcData,
        poolAddressScript,
        extraNoncePlaceholder,
        reward,
        txMessages,
        recipients
    )

    this.serializeCoinbase = function(extraNonce1, extraNonce2){
        return Buffer.concat([
            this.generationTransaction[0],
            extraNonce1,
            extraNonce2,
            this.generationTransaction[1]
        ])
    }

    //AION Block header specialization
    this.serializeHeader = function(merkleRoot, nTime, nonce){
        
        var header =  new Buffer(528)
        var position = 0

        //Serialize header based on equihash H(I || V || ....)
        //I - 496 bytes, V - 32 bytes 
        
        //console.log("Serialize Header nTime: " + nTime)

        header.write(rpcData.blockHeader.parentHash, position+=0, 32, 'hex'); //Parent Hash - 32 bytes
        header.write(rpcData.blockHeader.coinBase, position+=32, 32, 'hex'); //Coinbase - 32 bytes
        header.write(rpcData.blockHeader.stateRoot, position+=32, 32, 'hex'); //StateRoot - 32 bytes
        header.write(rpcData.blockHeader.txTrieRoot, position+=32, 32, 'hex'); //TxTrieRoot - 32 bytes
        header.write(rpcData.blockHeader.receiptTrieRoot, position+=32, 32, 'hex'); //RecieptTrieRoot - 32 bytes
        header.write(rpcData.blockHeader.logsBloom, position+=32, 256, 'hex'); //LogsBloom - 256 bytes
        header.write(rpcData.blockHeader.difficulty, position+=256, 16, 'hex'); //Difficulty - 16 bytes
        
        //header.write(rpcData.blockHeader.timestamp, position+=16, 8, 'hex'); //Timestamp - 16 bytes
        // replace timestamp in blockheader with updated mining timestamp.
        header.write(nTime, position+=16, 8, 'hex'); //Timestamp - 16 bytes
        
        header.write(rpcData.blockHeader.number, position+=8, 8, 'hex'); //Block Number - 8 bytes
        header.write(rpcData.blockHeader.extraData, position+=8, 32, 'hex'); //ExtraData - 32 bytes
        header.write(rpcData.blockHeader.energyConsumed, position+=32, 8, 'hex'); //Energy Consumed - 32 bytes
        header.write(rpcData.blockHeader.energyLimit, position+=8, 8, 'hex'); //Energy Limit - 8 bytes
        header.write(nonce, position+=8, 32, 'hex'); //Nonce - 32 bytes

        return header
    }

    //AION Block header specialization - TO FINISH TOMORROW
    this.serializeHeaderTarget = function(nonce, soln, nTime){
        
        var header = Buffer.alloc(1936)
        var position = 0
        
        header.write(rpcData.blockHeader.parentHash, position+=0, 32, 'hex'); //Parent Hash
        header.write(rpcData.blockHeader.coinBase, position+=32, 32, 'hex'); //Coinbase
        header.write(rpcData.blockHeader.stateRoot, position+=32, 32, 'hex'); //StateRoot
        header.write(rpcData.blockHeader.txTrieRoot, position+=32, 32, 'hex'); //TxTrieRoot
        header.write(rpcData.blockHeader.receiptTrieRoot, position+=32, 32, 'hex'); //RecieptTrieRoot
        header.write(rpcData.blockHeader.logsBloom, position+=32, 256, 'hex'); //LogsBloom
        header.write(rpcData.blockHeader.difficulty, position+=256, 16, 'hex');
        //header.write(rpcData.blockHeader.timestamp, position+=16, 8, 'hex');
        header.write(nTime, position+=16, 8, 'hex'); //Timestamp - 16 bytes
        header.write(rpcData.blockHeader.number, position+=8, 8, 'hex'); //Block Number
        header.write(rpcData.blockHeader.extraData, position+=8, 32, 'hex'); //ExtraData
        header.write(nonce, position+=32, 32, 'hex'); //Nonce
        header.write(soln.slice(6), position+=32, 1408, 'hex'); //Solution
        header.write(rpcData.blockHeader.energyConsumed, position+=1408, 8, 'hex'); //Energy Consumed
        header.write(rpcData.blockHeader.energyLimit, position+=8, 8, 'hex'); //Energy Limit

        return header
    }


    this.serializeBlock = function(header, coinbase){
        return Buffer.concat([
            header,

            util.varIntBuffer(this.rpcData.transactions.length + 1),
            coinbase,
            this.transactionData,

            getVoteData(),

            //POS coins require a zero byte appended to block which the daemon replaces with the signature
            new Buffer(reward === 'POS' ? [0] : [])
        ])
    }

    this.registerSubmit = function(extraNonce1, extraNonce2, nTime, nonce){
        var submission = extraNonce1 + extraNonce2 + nTime + nonce
        if (submits.indexOf(submission) === -1){
            submits.push(submission)
            return true
        }
        return false
    }

    this.getJobParams = function(){
        if (!this.jobParams){
            this.jobParams = [

                //Aion Job Params
                this.jobId,
                this.rpcData.blockHeader.parentHash,
                this.rpcData.blockHeader.coinBase,
                this.rpcData.blockHeader.stateRoot,
                this.rpcData.blockHeader.txTrieRoot,
                this.rpcData.blockHeader.receiptTrieRoot,
                this.rpcData.blockHeader.logsBloom,
                this.rpcData.blockHeader.difficulty,
                this.rpcData.blockHeader.timestamp,
                this.rpcData.blockHeader.number,
                this.rpcData.blockHeader.extraData,
                this.rpcData.blockHeader.energyConsumed,
                this.rpcData.blockHeader.energyLimit,
                true,
                this.rpcData.target
            ]
        }
        return this.jobParams
    }
}
