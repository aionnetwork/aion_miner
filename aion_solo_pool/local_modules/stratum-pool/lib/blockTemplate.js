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
    
    this.serializeCoinbase = function(extraNonce1, extraNonce2){
        return Buffer.concat([
            this.generationTransaction[0],
            extraNonce1,
            extraNonce2,
            this.generationTransaction[1]
        ])
    }

    //AION Block header serialization
    this.serializeHeader = function(merkleRoot, nTime, nonce){
        
        // Double hash serialization header

        var header =  new Buffer(72)
        var position = 0

        //I - 40 bytes (Partial hash + timestamp), V - 32 bytes
        header.write(rpcData.partialHash, position, 32, 'hex');
        header.write(nTime, position+=32, 8, 'hex');
        header.write(nonce, position+=8, 32, 'hex');

        return header
    }

    //AION Block header serialization
    this.serializeHeaderTarget = function(nonce, soln, nTime){

        var header = Buffer.alloc(1480);
        var position = 0;

        header.write(rpcData.partialHash, position, 32, 'hex'); //Partial Hash
        header.write(nTime, position+=32, 8, 'hex'); //Timestamp
        header.write(nonce, position+=8, 32, 'hex'); //Nonce
        header.write(soln.slice(6), position+=32, 1408, 'hex'); //Solution
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
                true, // Clean job
                this.rpcData.target,
                this.rpcData.partialHash
            ]
        }
        return this.jobParams
    }
}
