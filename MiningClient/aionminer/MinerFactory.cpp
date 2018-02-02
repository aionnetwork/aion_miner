#include "MinerFactory.h"

#include <thread>

extern int use_avx;
extern int use_avx2;

MinerFactory::~MinerFactory() {
	ClearAllSolvers();
}

std::vector<ISolver *> MinerFactory::GenerateSolvers(int cpu_threads,
		int cuda_count, int* cuda_en, int* cuda_b, int* cuda_t) {
	std::vector<ISolver *> solversPointers;

	for (int i = 0; i < cuda_count; ++i) {
		solversPointers.push_back(
				GenCUDASolver(cuda_en[i], cuda_b[i], cuda_t[i]));
	}

	bool hasGpus = solversPointers.size() > 0;
	if (cpu_threads < 0) {
		cpu_threads = std::thread::hardware_concurrency();
		if (cpu_threads < 1)
			cpu_threads = 1;
		else if (hasGpus)
			--cpu_threads; // decrease number of threads if there are GPU workers
	}

	for (int i = 0; i < cpu_threads; ++i) {
		solversPointers.push_back(GenCPUSolver(use_avx2));
	}

	return solversPointers;
}

void MinerFactory::ClearAllSolvers() {
	for (ISolver * ds : _solvers) {
		if (ds != nullptr) {
			delete ds;
		}
	}
	_solvers.clear();
}

ISolver * MinerFactory::GenCPUSolver(int use_opt) {
	// TODO fix dynamic linking on Linux
	_solvers.push_back(new CPUSolverTromp(use_opt));
	return _solvers.back();

}

ISolver * MinerFactory::GenCUDASolver(int dev_id, int blocks,
		int threadsperblock) {
	_solvers.push_back(new CUDASolverTromp(dev_id, blocks, threadsperblock));
	return _solvers.back();

}
