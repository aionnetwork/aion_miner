#include <iostream>
#include <functional>
#include <vector>

#include "equi_miner_210.h"
#include "cpu_tromp.hpp"

void CPU_TROMP::start(CPU_TROMP& device_context) {
//void CPU_TROMP::start() {
}

void CPU_TROMP::stop(CPU_TROMP& device_context) {
//void CPU_TROMP::stop() {
}

void CPU_TROMP::solve(const char *tequihash_header,
		unsigned int tequihash_header_len, const char* nonce,
		unsigned int nonce_len, std::function<bool()> cancelf,
		std::function<
				void(const std::vector<uint32_t>&, size_t, const unsigned char*)> solutionf,
		std::function<void(void)> hashdonef,
		CPU_TROMP& device_context) {

	equi eq(1, tequihash_header_len, nonce_len);
	eq.setnonce(tequihash_header, tequihash_header_len, nonce, nonce_len);
	eq.digit0(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 1
	if (cancelf())
		return;
	eq.digit1(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 2
	if (cancelf())
		return;
	eq.digit2(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 3
	if (cancelf())
		return;
	eq.digit3(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 4
	if (cancelf())
		return;
	eq.digit4(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 5
	if (cancelf())
		return;
	eq.digit5(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 6
	if (cancelf())
		return;
	eq.digit6(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 7
	if (cancelf())
		return;
	eq.digit7(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 8
	if (cancelf())
		return;
	eq.digit8(0);
	eq.bfull = eq.hfull = 0;

	//Process Digit 9
	if (cancelf())
		return;
	eq.digit9(0);
	eq.bfull = eq.hfull = 0;

	if (cancelf())
		return;

	for (unsigned s = 0; s < eq.nsols; s++) {
		std::vector<uint32_t> index_vector(PROOFSIZE);
		for (u32 i = 0; i < PROOFSIZE; i++) {
			index_vector[i] = eq.sols[s][i];
		}
		solutionf(index_vector, DIGITBITS, nullptr);
		if (cancelf())
			return;
	}

	hashdonef();
}
