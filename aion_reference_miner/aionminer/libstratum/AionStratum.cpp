// Copyright (c) 2017 Aion Foundation (ross@nuco.io)
// Copyright (c) 2016 Jack Grigg <jack@z.cash>
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "version.h"
#include "AionStratum.h"

#include "utilstrencodings.h"
#include "streams.h"

#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <boost/thread/exceptions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/circular_buffer.hpp>
#include "speed.hpp"
#include <cstdint>
#include "../../blake2/blake2.h"
#include <boost/static_assert.hpp>

typedef uint32_t eh_index;

#define BOOST_LOG_CUSTOM(sev, pos) BOOST_LOG_TRIVIAL(sev) << "miner#" << pos << " | "

void CompressArray(const unsigned char* in, size_t in_len, unsigned char* out,
		size_t out_len, size_t bit_len, size_t byte_pad) {
	assert(bit_len >= 8);
	assert(8 * sizeof(uint32_t) >= 7 + bit_len);

	size_t in_width { (bit_len + 7) / 8 + byte_pad };
	assert(out_len == bit_len * in_len / (8 * in_width));

	uint32_t bit_len_mask { ((uint32_t) 1 << bit_len) - 1 };

	// The acc_bits least-significant bits of acc_value represent a bit sequence
	// in big-endian order.
	size_t acc_bits = 0;
	uint32_t acc_value = 0;

	size_t j = 0;
	for (size_t i = 0; i < out_len; i++) {
		// When we have fewer than 8 bits left in the accumulator, read the next
		// input element.
		if (acc_bits < 8) {
			acc_value = acc_value << bit_len;
			for (size_t x = byte_pad; x < in_width; x++) {
				acc_value = acc_value
						| ((
						// Apply bit_len_mask across byte boundaries
						in[j + x]
								& ((bit_len_mask >> (8 * (in_width - x - 1)))
										& 0xFF)) << (8 * (in_width - x - 1))); // Big-endian
			}
			j += in_width;
			acc_bits += bit_len;
		}

		acc_bits -= 8;
		out[i] = (acc_value >> acc_bits) & 0xFF;
	}
}

void EhIndexToArray(const eh_index i, unsigned char* array) {
	BOOST_STATIC_ASSERT(sizeof(eh_index) == 4);
	eh_index bei = htobe32(i);
	memcpy(array, &bei, sizeof(eh_index));
}

std::vector<unsigned char> GetMinimalFromIndices(std::vector<eh_index> indices,
		size_t cBitLen) {
	assert(((cBitLen + 1) + 7) / 8 <= sizeof(eh_index));
	size_t lenIndices { indices.size() * sizeof(eh_index) };
	size_t minLen { (cBitLen + 1) * lenIndices / (8 * sizeof(eh_index)) };
	size_t bytePad { sizeof(eh_index) - ((cBitLen + 1) + 7) / 8 };
	std::vector<unsigned char> array(lenIndices);
	for (int i = 0; i < indices.size(); i++) {
		EhIndexToArray(indices[i], array.data() + (i * sizeof(eh_index)));
	}
	std::vector<unsigned char> ret(minLen);
	CompressArray(array.data(), lenIndices, ret.data(), minLen, cBitLen + 1,
			bytePad);
	return ret;
}

void static AionMinerThread(AionMiner* miner, int size, int pos,
		ISolver *solver) {
	BOOST_LOG_CUSTOM(info, pos) << "Starting thread #" << pos << " ("
			<< solver->getname() << ") " << solver->getdevinfo();

	std::shared_ptr < std::mutex > m_zmt(new std::mutex);
	ABlockHeader header;
	arith_uint256 space;
	size_t offset;
	arith_uint256 inc;
	arith_uint256 target;
	std::string target_str;
	std::string jobId;
	std::string nTime;
	std::atomic_bool workReady { false };
	std::atomic_bool cancelSolver { false };
	std::atomic_bool pauseMining { false };

	miner->NewJob.connect(
			NewJob_t::slot_type(
					[&m_zmt, &header, &space, &offset, &inc, &target, &workReady, &cancelSolver, pos, &pauseMining, &jobId, &nTime]
					(const AionJob* job) mutable {
						std::lock_guard<std::mutex> lock {*m_zmt.get()};
						if (job) {
							BOOST_LOG_CUSTOM(debug, pos) << "Loading new job #" << job->jobId();
							jobId = job->jobId();
							nTime = job->time;
							header = job->header;
							space = job->nonce2Space;
							offset = job->nonce1Size * 4; // Hex length to bit length
							inc = job->nonce2Inc;
							target = job->serverTarget;
							pauseMining.store(false);
							workReady.store(true);
							if (job->clean) {
								cancelSolver.store(true);
							}

						} else {
							workReady.store(false);
							cancelSolver.store(true);
							pauseMining.store(true);
						}
					}).track_foreign(m_zmt)); // So the signal disconnects when the mining thread exits

	try {

		solver->start();

		while (true) {
			// Wait for work
			bool expected;
			do {
				expected = true;
				if (!miner->minerThreadActive[pos])
					throw boost::thread_interrupted();
				//boost::this_thread::interruption_point();
				std::this_thread::sleep_for(std::chrono::milliseconds(1000));
			} while (!workReady.compare_exchange_weak(expected, false));
			// TODO change atomically with workReady
			cancelSolver.store(false);

			// Calculate nonce limits
			arith_uint256 nonce;
			arith_uint256 nonceEnd;
			ABlockHeader actualHeader;
			std::string actualJobId;
			std::string actualTime;
			arith_uint256 actualTarget;
			size_t actualNonce1size;
			{
				std::lock_guard < std::mutex > lock { *m_zmt.get() };
				arith_uint256 baseNonce = UintToArith256(header.nNonce);
				arith_uint256 add(pos);
				nonce = baseNonce | (add << (8 * 19));
				nonceEnd = baseNonce | ((add + 1) << (8 * 19));
				//nonce = baseNonce + ((space/size)*pos << offset);
				//nonceEnd = baseNonce + ((space/size)*(pos+1) << offset);

				// save job id and time
				actualHeader = header;
				actualJobId = jobId;
				actualTime = nTime;
				actualNonce1size = offset / 4;
				actualTarget = target;
			}

			// I = the block header minus nonce and solution.
			CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
			{
				//std::lock_guard<std::mutex> lock{ *m_zmt.get() };
				AEquihashInput I { actualHeader };
				ss << I;

				// printf("Equihash Input Fields: \n");
				// BOOST_LOG_CUSTOM(debug, pos) << "Parent Hash:  " << actualHeader.parentHash.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "CoinBase:  " << actualHeader.coinBase.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "State root:  " << actualHeader.stateRoot.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "txTrie:  " << actualHeader.txTrie.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "ReceiptTreeRoot :  " << actualHeader.receiptTreeRoot.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "LogsBloom:  " << actualHeader.logsBloom.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "Difficulty:  " << actualHeader.difficulty.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "Timestamp:  " << actualHeader.timeStamp;
				// BOOST_LOG_CUSTOM(debug, pos) << "NUmber:  " << actualHeader.number;
				// BOOST_LOG_CUSTOM(debug, pos) << "extraData:  " << actualHeader.extraData.ToString();
				// BOOST_LOG_CUSTOM(debug, pos) << "energyConsumed:  " << actualHeader.energyConsumed;
				// BOOST_LOG_CUSTOM(debug, pos) << "energyLimit:  " << actualHeader.energyLimit;

			}

			char *tequihash_header = (char *) &ss[0];
			unsigned int tequihash_header_len = ss.size();

			// Start working
			while (true) {
				BOOST_LOG_CUSTOM(debug, pos)
						<< "Running Equihash solver with nNonce = "
						<< nonce.ToString();

				auto bNonce = ArithToUint256(nonce);

				std::function<
						void(const std::vector<uint32_t>&, size_t,
								const unsigned char*)> solutionFound =
						[&actualHeader, &bNonce, &actualTarget, &miner, pos, &actualJobId, &actualTime, &actualNonce1size]
						(const std::vector<uint32_t>& index_vector, size_t cbitlen, const unsigned char* compressed_sol)
						{
							actualHeader.nNonce = bNonce;
							if (compressed_sol)
							{
								//Change vector to 1408 to accomodate new 210,9 parameters
								actualHeader.nSolution = std::vector<unsigned char>(1408);
								for (size_t i = 0; i < cbitlen; ++i)
								actualHeader.nSolution[i] = compressed_sol[i];
							}
							else
							actualHeader.nSolution = GetMinimalFromIndices(index_vector, cbitlen);

							speed.AddSolution();

							BOOST_LOG_CUSTOM(debug, pos) << "Checking solution against target...";
							BOOST_LOG_CUSTOM(debug, pos) << "Compressed solution size: " << actualHeader.nSolution.size();

							//Create a new vector with size equaling a full header
							std::vector<uint8_t> headerBytes;
							//Reserve header + nonce + solution size
							headerBytes.reserve(actualHeader.HEADER_SIZE + 1408 + 32);

							headerBytes.insert(headerBytes.end(), actualHeader.parentHash.begin(), actualHeader.parentHash.end());
							headerBytes.insert(headerBytes.end(), actualHeader.coinBase.begin(), actualHeader.coinBase.end());
							headerBytes.insert(headerBytes.end(), actualHeader.stateRoot.begin(), actualHeader.stateRoot.end());
							headerBytes.insert(headerBytes.end(), actualHeader.txTrie.begin(), actualHeader.txTrie.end());
							headerBytes.insert(headerBytes.end(), actualHeader.receiptTreeRoot.begin(), actualHeader.receiptTreeRoot.end());
							headerBytes.insert(headerBytes.end(), actualHeader.logsBloom.begin(), actualHeader.logsBloom.end());
							headerBytes.insert(headerBytes.end(), actualHeader.difficulty.begin(), actualHeader.difficulty.end());
							uint8_t * timestamp = (uint8_t*)&actualHeader.timeStamp;
							for(int i = 0; i < 8; i++) {
								headerBytes.push_back(*(timestamp+i));
							}

							uint8_t * number = (uint8_t*)&actualHeader.number;
							for(int i = 0; i < 8; i++) {
								headerBytes.push_back(*(number+i));
							}

							headerBytes.insert(headerBytes.end(), actualHeader.extraData.begin(), actualHeader.extraData.end());
							headerBytes.insert(headerBytes.end(), actualHeader.nNonce.begin(), actualHeader.nNonce.end());
							headerBytes.insert(headerBytes.end(), actualHeader.nSolution.begin(), actualHeader.nSolution.end());

							uint8_t * energyConsumed = (uint8_t*)&actualHeader.energyConsumed;
							for(int i = 0; i < 8; i++) {
								headerBytes.push_back(*(energyConsumed + i));
							}

							uint8_t * energyLimit = (uint8_t*)&actualHeader.energyLimit;
							for(int i = 0; i < 8; i++) {
								headerBytes.push_back(*(energyLimit+i));
							}

							BOOST_LOG_CUSTOM(debug, pos) << "headerBytes size: " << headerBytes.size();

							unsigned char * headerInput = &headerBytes[0];
							blake2b_state target_state;

							blake2b_param P[1];
							P->digest_length = 32;
							P->key_length = 0;
							P->fanout = 1;
							P->depth = 1;
							P->leaf_length = 0;
							P->node_offset = 0;
							P->node_depth = 0;
							P->inner_length = 0;
							memset(P->reserved, 0, sizeof(P->reserved));
							memset(P->salt, 0, sizeof(P->salt));
							memset(P->personal, 0, sizeof(P->personal));
							blake2b_init_param(&target_state, P);

							blake2b_update(&target_state, headerInput, headerBytes.size());

							//Generate 32 byte hash of the header (with solution and nonce)
							unsigned char hash[32];
							blake2b_final(&target_state, hash, 32);

							std::string targetHex = actualTarget.GetHex();

							std::vector<unsigned char> bytes;
							bytes.reserve(targetHex.size() / 2);
							for (std::string::size_type i = 0, i_end = targetHex.size(); i < i_end; i += 2)
							{
								unsigned byte;
								std::istringstream hex_byte(targetHex.substr(i, 2));
								hex_byte >> std::hex >> byte;
								bytes.push_back(static_cast<unsigned char>(byte));
							}

							unsigned char * targetBytes = &bytes[0];

							int targetComp = memcmp(hash, targetBytes, 32);

							std::vector<uint8_t> h;
							h.assign(hash, hash+32);
							uint256 hdrHash = base_blob<256>(h);

							if(targetComp >= 0) {
								//Hash of the header was greater than TargetBytes
								BOOST_LOG_CUSTOM(debug, pos) << "Hash of header was larger than target";
								return;
							}

							// Found a solution
							BOOST_LOG_CUSTOM(debug, pos) << "Found a valid solution";

							EquihashSolution solution {actualHeader.nNonce, actualHeader.nSolution, actualTime, actualNonce1size};
							miner->submitSolution(solution, actualJobId);
						};

				std::function < bool() > cancelFun = [&cancelSolver]() {
					return cancelSolver.load();
				};

				std::function<void(void)> hashDone = []() {
					speed.AddHash();
				};

				// Check for stop
				if (!miner->minerThreadActive[pos])
					throw boost::thread_interrupted();

				solver->solve(tequihash_header, tequihash_header_len,
						(const char*) bNonce.begin(), bNonce.size(), cancelFun,
						solutionFound, hashDone);

				//boost::this_thread::interruption_point();

				// Update nonce
				nonce += inc;

				if (nonce == nonceEnd) {
					break;
				}

				// Check for new work
				if (workReady.load()) {
					BOOST_LOG_CUSTOM(debug, pos)
							<< "New work received, dropping current work";
					break;
				}

				if (pauseMining.load()) {
					BOOST_LOG_CUSTOM(debug, pos) << "Mining paused";
					break;
				}
			}
		}
	} catch (const boost::thread_interrupted&) {
		//throw;
	} catch (const std::runtime_error &e) {
		BOOST_LOG_CUSTOM(error, pos) << e.what();
		exit(0);
	}

	try {
		solver->stop();
	} catch (const std::runtime_error &e) {
		BOOST_LOG_CUSTOM(error, pos) << e.what();
	}

	BOOST_LOG_CUSTOM(info, pos) << "Thread #" << pos << " ended ("
			<< solver->getname() << ")";
}

AionJob* AionJob::clone() const {
	AionJob* ret = new AionJob();
	ret->job = job;
	ret->header = header;
	ret->time = time;
	ret->nonce1Size = nonce1Size;
	ret->nonce2Space = nonce2Space;
	ret->nonce2Inc = nonce2Inc;
	ret->serverTarget = serverTarget;
	ret->serverTarget_str = serverTarget_str;
	ret->clean = clean;
	return ret;
}

void AionJob::setTarget(std::string target) {
	if (target.size() > 0) {
		serverTarget = UintToArith256(uint256S(target));
		serverTarget_str = target;
	} else {
		BOOST_LOG_TRIVIAL(debug)
				<< "miner | New job but no server target, assuming powLimit";
		serverTarget =
				UintToArith256(
						uint256S(
								"ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));
	}
}

std::string AionJob::getSubmission(const EquihashSolution* solution) {
	CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
	ss << solution->nonce;
	ss << solution->solution;
	std::string strHex = HexStr(ss.begin(), ss.end());

	std::stringstream stream;
	stream << "\"" << job;
	stream << "\",\"" << time;
	stream << "\",\"" << strHex.substr(nonce1Size, 64 - nonce1Size);
	stream << "\",\"" << strHex.substr(64);
	stream << "\"";
	return stream.str();
}

AionMiner::AionMiner(const std::vector<ISolver *> &i_solvers) :
		minerThreads { nullptr } {
	m_isActive = false;
	solvers = i_solvers;
	nThreads = solvers.size();
}

AionMiner::~AionMiner() {
	stop();
}

std::string AionMiner::userAgent() {
	return "nheqminer/" STANDALONE_MINER_VERSION;
}

void AionMiner::start() {
	if (minerThreads) {
		stop();
	}

	m_isActive = true;

	minerThreads = new std::thread[nThreads];
	minerThreadActive = new bool[nThreads];

	// sort solvers CPU, CUDA, OPENCL
	std::sort(solvers.begin(), solvers.end(),
			[](const ISolver* a, const ISolver* b) {return a->GetType() < b->GetType();});

	// start solvers
	// #1 start cpu threads
	// #2 start CUDA threads
	// #3 start OPENCL threads
	for (int i = 0; i < solvers.size(); ++i) {
		minerThreadActive[i] = true;
		minerThreads[i] = std::thread(
				boost::bind(&AionMinerThread, this, nThreads, i, solvers[i]));
		if (solvers[i]->GetType() == SolverType::CPU) {
			// todo: linux set low priority
		}
	}

	//for ( ; )
	//{
	//
	//}

//	for (; i < (cpu_contexts.size() + cuda_contexts.size()); ++i) {
//		minerThreadActive[i] = true;
//		minerThreads[i] = std::thread(
//				boost::bind(
//						&ZcashMinerThread<CPUSolver, CUDASolver, OPENCLSolver,
//								CUDASolver>, this, nThreads, i,
//						*cuda_contexts.at(i - cpu_contexts.size())));
//	}

	//
	//for (; i < (cpu_contexts.size() + cuda_contexts.size() + opencl_contexts.size()); ++i)
	//{
	//    minerThreadActive[i] = true;
	//    minerThreads[i] = std::thread(boost::bind(&ZcashMinerThread<CPUSolver, CUDASolver, OPENCLSolver, OPENCLSolver>,
	//        this, nThreads, i, *opencl_contexts.at(i - cpu_contexts.size() - cuda_contexts.size())));
	//}

	///*minerThreads = new boost::thread_group();
	//for (int i = 0; i < nThreads; i++) {
	//    minerThreads->create_thread(boost::bind(&ZcashMinerThread, this, nThreads, i));
	//}*/

	speed.Reset();
}

void AionMiner::stop() {
	m_isActive = false;
	if (minerThreads) {
		for (int i = 0; i < nThreads; i++)
			minerThreadActive[i] = false;
		for (int i = 0; i < nThreads; i++)
			minerThreads[i].join();
		delete[] minerThreads;
		minerThreads = nullptr;
		delete[] minerThreadActive;
		minerThreadActive = nullptr;
	}
	/*if (minerThreads) {
	 minerThreads->interrupt_all();
	 delete minerThreads;
	 minerThreads = nullptr;
	 }*/
}

void AionMiner::setServerNonce(const std::string& n1str) {
	//auto n1str = params[1].get_str();
	BOOST_LOG_TRIVIAL(info) << "miner | Extranonce is " << n1str;
	std::vector<unsigned char> nonceData(ParseHex(n1str));
	while (nonceData.size() < 32) {
		nonceData.push_back(0);
	}
	CDataStream ss(nonceData, SER_NETWORK, PROTOCOL_VERSION);
	ss >> nonce1;

	//BOOST_LOG_TRIVIAL(info) << "miner | Full nonce " << nonce1.ToString();

	nonce1Size = n1str.size();
	size_t nonce1Bits = nonce1Size * 4; // Hex length to bit length
	size_t nonce2Bits = 256 - nonce1Bits;

	nonce2Space = 1;
	nonce2Space <<= nonce2Bits;
	nonce2Space -= 1;

	nonce2Inc = 1;
	nonce2Inc <<= nonce1Bits;
}

AionJob* AionMiner::parseJob(const Array& params) {
	if (params.size() < 2) {
		throw std::logic_error("Invalid job params");
	}

	AionJob* ret = new AionJob();
	ret->job = params[0].get_str(); //JobId

	if (params.size() < 14) {
		throw std::logic_error("Invalid job params");
	}

	std::stringstream ssHeader;
	ssHeader
			<< params[1].get_str()    //Parent Hash
			<< params[2].get_str() //Coinbase
			<< params[3].get_str() //State Root
			<< params[4].get_str() //TxTrie Root
			<< params[5].get_str() //RecieptTrie Root
			<< params[6].get_str() //LogsBloom
			<< params[7].get_str() //Difficulty
			<< params[8].get_str() //Timestamp
			<< params[9].get_str() //Block Number
			<< params[10].get_str() //extradata
			<< params[11].get_str() //Energy Consumed
			<< params[12].get_str() //Energy Limit

			// Empty nonce
			<< "0000000000000000000000000000000000000000000000000000000000000000"
			<< "00"; // Empty solution
	auto strHexHeader = ssHeader.str();
	std::vector<unsigned char> headerData(ParseHex(strHexHeader));

	CDataStream ss(headerData, SER_NETWORK, PROTOCOL_VERSION);
	try {

		BOOST_LOG_CUSTOM(debug, 0) << "Param 1: " << params[1].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 2: " << params[2].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 3: " << params[3].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 4: " << params[4].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 5: " << params[5].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 6: " << params[6].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 7: " << params[7].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 8: " << params[8].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 9: " << params[9].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 10: " << params[10].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 11: " << params[11].get_str();
		BOOST_LOG_CUSTOM(debug, 0) << "Param 12: " << params[12].get_str();

		ss >> ret->header;

	} catch (const std::ios_base::failure& e) {
		std::cout << "Error caught: " << e.what() << "\n";
		throw std::logic_error(
				"AionMiner::parseJob(): Invalid block header parameters");
	}

	ret->time = params[8].get_str();
	ret->clean = params[13].get_bool();

	ret->header.nNonce = nonce1;
	ret->nonce1Size = nonce1Size;
	ret->nonce2Space = nonce2Space;
	ret->nonce2Inc = nonce2Inc;

	ret->setTarget(params[14].get_str());

	return ret;
}

void AionMiner::setJob(AionJob* job) {
	NewJob(job);
}

void AionMiner::onSolutionFound(
		const std::function<bool(const EquihashSolution&, const std::string&)> callback) {
	solutionFoundCallback = callback;
}

void AionMiner::submitSolution(const EquihashSolution& solution,
		const std::string& jobid) {
	solutionFoundCallback(solution, jobid);
	speed.AddShare();
}

void AionMiner::acceptedSolution(bool stale) {
	speed.AddShareOK();
}

void AionMiner::rejectedSolution(bool stale) {
}

void AionMiner::failedSolution() {
}

std::mutex benchmark_work;
std::vector<uint256*> benchmark_nonces;
std::atomic_int benchmark_solutions;

bool benchmark_solve_equihash(const ABlock& pblock,
		const char *tequihash_header, unsigned int tequihash_header_len,
		ISolver *solver) {
	benchmark_work.lock();
	if (benchmark_nonces.empty()) {
		benchmark_work.unlock();
		return false;
	}
	uint256* nonce = benchmark_nonces.front();
	benchmark_nonces.erase(benchmark_nonces.begin());
	benchmark_work.unlock();

	BOOST_LOG_TRIVIAL(debug) << "Testing, nonce = " << nonce->ToString();

	std::function<
			void(const std::vector<uint32_t>&, size_t, const unsigned char*)> solutionFound =
			[&pblock, &nonce]
			(const std::vector<uint32_t>& index_vector, size_t cbitlen, const unsigned char* compressed_sol)
			{
				ABlockHeader hdr = pblock.GetBlockHeader();
				hdr.nNonce = *nonce;

				if (compressed_sol)
				{
					hdr.nSolution = std::vector<unsigned char>(1408);
					for (size_t i = 0; i < cbitlen; ++i)
					hdr.nSolution[i] = compressed_sol[i];
				}
				else
				hdr.nSolution = GetMinimalFromIndices(index_vector, cbitlen);

				++benchmark_solutions;
			};

	solver->solve(tequihash_header, tequihash_header_len,
			(const char*) nonce->begin(), nonce->size(), []() {return false;},
			solutionFound, []() {});

	delete nonce;

	return true;
}

int benchmark_thread(int tid, ISolver *solver) {
	BOOST_LOG_TRIVIAL(debug) << "Thread #" << tid << " started ("
			<< solver->getname() << ")";

	try {
		ABlock pblock;
		AEquihashInput I { pblock };
		CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
		ss << I;

		const char *tequihash_header = (char *) &ss[0];
		unsigned int tequihash_header_len = ss.size();

		solver->start();

		while (benchmark_solve_equihash(pblock, tequihash_header,
				tequihash_header_len, solver)) {
		}

		solver->stop();
	} catch (const std::runtime_error &e) {
		BOOST_LOG_TRIVIAL(error) << e.what();
		exit(0);
		return 0;
	}

	BOOST_LOG_TRIVIAL(debug) << "Thread #" << tid << " ended ("
			<< solver->getname() << ")";

	return 0;
}

void Solvers_doBenchmark(int hashes, const std::vector<ISolver *> &solvers) {
	// generate array of various nonces
	std::srand(std::time(0));
	benchmark_nonces.push_back(new uint256());
	benchmark_nonces.back()->begin()[31] = 1;
	for (int i = 0; i < (hashes - 1); ++i) {
		benchmark_nonces.push_back(new uint256());
		for (unsigned int i = 0; i < 32; ++i)
			benchmark_nonces.back()->begin()[i] = std::rand() % 256;
	}
	benchmark_solutions = 0;

	size_t total_hashes = benchmark_nonces.size();

	// log what is benchmarking
	for (ISolver* solver : solvers) {
		if (solver->GetType() == SolverType::CPU) {
			BOOST_LOG_TRIVIAL(info) << "Benchmarking CPU worker ("
					<< solver->getname() << ") " << solver->getdevinfo();
		} else if (solver->GetType() == SolverType::CUDA) {
			BOOST_LOG_TRIVIAL(info) << "Benchmarking CUDA worker ("
					<< solver->getname() << ") " << solver->getdevinfo();
		}
	}

	int nThreads = solvers.size();
	std::thread* bthreads = new std::thread[nThreads];

	benchmark_work.lock();
	// bind benchmark threads
	for (int i = 0; i < solvers.size(); ++i) {
		bthreads[i] = std::thread(
				boost::bind(&benchmark_thread, i, solvers[i]));
	}
#ifdef WIN32
	// TODO get back to this sleep
	Sleep(1000);
#else
	sleep(1);
#endif

	BOOST_LOG_TRIVIAL(info)
			<< "Benchmark starting... this may take several minutes, please wait...";

	benchmark_work.unlock();
	auto start = std::chrono::high_resolution_clock::now();

	for (int i = 0; i < nThreads; ++i)
		bthreads[i].join();

	auto end = std::chrono::high_resolution_clock::now();

	uint64_t msec = std::chrono::duration_cast < std::chrono::milliseconds
			> (end - start).count();

	size_t hashes_done = total_hashes - benchmark_nonces.size();

	BOOST_LOG_TRIVIAL(info) << "Benchmark done!";
	BOOST_LOG_TRIVIAL(info) << "Total time : " << msec << " ms";
	BOOST_LOG_TRIVIAL(info) << "Total iterations: " << hashes_done;
	BOOST_LOG_TRIVIAL(info) << "Total solutions found: " << benchmark_solutions;
	BOOST_LOG_TRIVIAL(info) << "Speed: "
			<< ((double) hashes_done * 1000 / (double) msec) << " I/s";
	BOOST_LOG_TRIVIAL(info) << "Speed: "
			<< ((double) benchmark_solutions * 1000 / (double) msec)
			<< " Sols/s";
}
