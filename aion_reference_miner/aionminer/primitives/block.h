// Modified AION Foundation 2017-2018
// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

//#include "primitives/transaction.h"
#include "serialize.h"
#include "uint256.h"

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */

class ABlockHeader
{
public:
	// AION header

	/*
	Parent hash: 32 bytes
	CoinBase: 32 bytes
	StateRoot: 32 bytes
	TxTrie: 32 bytes
	ReceiptTrieRoot: 32 bytes
	logsBloom: 256 bytes
	Difficulty: 16 bytes
	Timestamp: 8 bytes
	Number: 8 bytes
	extraData: 32 bytes
	EnergyConsumed: 8 bytes
	EnergyLimit: 8 bytes

	Total: 484 bytes
	*/

	// static const size_t HEADER_SIZE = 32 + 32 + 32 + 32 + 32 + 256 + 16 + 8 + 8 + 32 + 8 + 8; // excluding Equihash solution and nonce

	// uint256 parentHash;
	// uint256 coinBase;
	// uint256 stateRoot;
	// uint256 txTrie;
	// uint256 receiptTreeRoot;
	// uint2048 logsBloom;
	// uint128 difficulty;
	// uint64_t timeStamp;
	// uint64_t number;
	// uint256 extraData;
	// uint64_t energyConsumed;
	// uint64_t energyLimit;

	// uint256 nNonce;
	// std::vector<unsigned char> nSolution;

	static const size_t HEADER_SIZE = 32 + 8; //Partial hash of header + timestamp (excluding nonce and solution)

	uint256 partialHash;
	uint64_t timeStamp;

	uint256 nNonce;
	std::vector<unsigned char> nSolution;

	ABlockHeader()
	{
		SetNull();
	}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {

		// READWRITE(parentHash);
		// READWRITE(coinBase);
		// READWRITE(stateRoot);
		// READWRITE(txTrie);
		// READWRITE(receiptTreeRoot);
		// READWRITE(logsBloom);
		// READWRITE(difficulty);
		// READWRITE(timeStamp);
		// READWRITE(number);
		// READWRITE(extraData);
		// READWRITE(energyConsumed);
		// READWRITE(energyLimit);

		READWRITE(partialHash);
		READWRITE(timeStamp);

		READWRITE(nNonce);
		READWRITE(nSolution);
	}

	void SetNull()
	{
		partialHash.SetNull();
		timeStamp = 0;
		// parentHash.SetNull();
        // coinBase.SetNull();
		// stateRoot.SetNull();
		// txTrie.SetNull();
		// receiptTreeRoot.SetNull();
		// logsBloom.SetNull();
		// difficulty.SetNull();
		// timeStamp = 0;
		// number = 0;
		// extraData.SetNull();
		// energyConsumed = 0;
		// energyLimit = 0;
		nNonce = uint256();
		nSolution.clear();
	}

	// bool IsNull() const
	// {
	// 	return (number == 0);
	// }

	uint256 GetHash() const;

	int64_t GetBlockTime() const
	{
		return timeStamp;
	}
};


class ABlock : public ABlockHeader
{
public:
	// network and disk
	//std::vector<CTransaction> vtx;

	// memory only
	mutable std::vector<uint256> vMerkleTree;

	ABlock()
	{
		SetNull();
	}

	ABlock(const ABlockHeader &header)
	{
		SetNull();
		*((ABlockHeader*)this) = header;
	}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
		READWRITE(*(ABlockHeader*)this);
		//READWRITE(vtx);
	}

	void SetNull()
	{
		ABlockHeader::SetNull();
		//vtx.clear();
		//vMerkleTree.clear();
	}

	ABlockHeader GetBlockHeader() const
	{
		ABlockHeader block;

		block.partialHash = partialHash;
		block.timeStamp = timeStamp;
		// block.parentHash = parentHash;
        // block.coinBase = coinBase;
		// block.stateRoot = stateRoot;
		// block.txTrie = txTrie;
		// block.receiptTreeRoot = receiptTreeRoot;
        // block.logsBloom = logsBloom;
		// block.difficulty = difficulty;
		// block.timeStamp = timeStamp;
		// block.number = number;
		// block.extraData = extraData;
		// block.energyConsumed = energyConsumed;
		// block.energyLimit = energyLimit;
		block.nNonce = nNonce;
		block.nSolution = nSolution;
		return block;
	}

		std::string ToString() const;
};


/**
* Custom serializer for ABlockHeader that omits the nonce and solution, for use
* as input to Equihash.
*/
class AEquihashInput : private ABlockHeader
{
public:
	AEquihashInput(const ABlockHeader &header)
	{
		ABlockHeader::SetNull();
		*((ABlockHeader*)this) = header;
	}

	ADD_SERIALIZE_METHODS;

	template <typename Stream, typename Operation>
	inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {

		// READWRITE(parentHash);
		// READWRITE(coinBase);
		// READWRITE(stateRoot);
		// READWRITE(txTrie);
		// READWRITE(receiptTreeRoot);
		// READWRITE(logsBloom);
		// READWRITE(difficulty);
		// READWRITE(timeStamp);
		// READWRITE(number);
		// READWRITE(extraData);
		// READWRITE(energyConsumed);
		// READWRITE(energyLimit);

		READWRITE(partialHash);
		READWRITE(timeStamp);
	}
};

#endif // BITCOIN_PRIMITIVES_BLOCK_H
