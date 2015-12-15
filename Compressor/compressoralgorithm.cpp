#include "compressor.h"
#include "compressoralgorithm.h"
#include "cudafitnessevaluator.h"
#include "utils.h"

using namespace std;
using namespace lossycompressor;

CompressorAlgorithm::CompressorAlgorithm(CompressorAlgorithmArgs* args)
: args(args) {
	cpuFitnessEvaluator = new CpuFitnessEvaluator(
		args->sourceWidth, 
		args->sourceHeight,
		args->diagramPointsCount,
		args->sourceImageData);

	if (args->useCuda) {
		fitnessEvaluator = new CudaFitnessEvaluator(
			args->sourceWidth,
			args->sourceHeight,
			args->diagramPointsCount,
			args->sourceImageData);
	}
	else {
		fitnessEvaluator = cpuFitnessEvaluator;
	}
}

CompressorAlgorithm::~CompressorAlgorithm() {
	if (fitnessEvaluator != cpuFitnessEvaluator) {
		delete cpuFitnessEvaluator;
	}
	delete fitnessEvaluator;
}

float CompressorAlgorithm::calculateFitness(VoronoiDiagram * diagram) {
	return fitnessEvaluator->calculateFitness(diagram);
}

int CompressorAlgorithm::compress(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {
	if (args->limitByTime) {
		Utils::recordTime(&computationStartTime);
	}
	else {
		fitnessEvaluator->resetFitnessCalculationCount();
	}

	return compressInternal(outputDiagram, colors, pixelPointAssignment);
}

bool CompressorAlgorithm::canContinueComputing() {
	if (args->limitByTime) {
		LARGE_INTEGER currentTime;
		Utils::recordTime(&currentTime);
		return Utils::calculateInterval(&computationStartTime, &currentTime)
			< args->maxComputationTimeSecs;
	}
	else {
		return fitnessEvaluator->getFitnessEvaluationsCount() < args->maxFitnessEvaluationCount;
	}
}

void CompressorAlgorithm::onBetterSolutionFound(float bestFitness) {
	printf("Found better solution with fitness %f\n", bestFitness);
}

void CompressorAlgorithm::onBestSolutionFound(float bestFitness) {
	printf("Found best solution with fitness %f\n", bestFitness);
}
