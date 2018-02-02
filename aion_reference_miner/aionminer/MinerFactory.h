#pragma once

#include <AvailableSolvers.h>

class MinerFactory {
public:
	MinerFactory() {
	}

	~MinerFactory();

	std::vector<ISolver *> GenerateSolvers(int cpu_threads, int cuda_count,
			int* cuda_en, int* cuda_b, int* cuda_t);
	void ClearAllSolvers();

private:
	std::vector<ISolver *> _solvers;

	ISolver * GenCPUSolver(int use_opt);
	ISolver * GenCUDASolver(int dev_id, int blocks, int threadsperblock);
};

