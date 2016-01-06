#include "compressor.h"
#include "compressoralgorithm.h"
#include "cudafitnessevaluator.h"
#include "utils.h"

using namespace std;
using namespace lossycompressor;

CompressorAlgorithm::CompressorAlgorithm(CompressorAlgorithm::Args* args)
: args(args) {
	cpuFitnessEvaluator = new CpuFitnessEvaluator(
		args->sourceWidth, 
		args->sourceHeight,
		args->diagramPointsCount,
		args->sourceImageData,
		args->sourceDataRowWidthInBytes);

	if (args->useCuda) {
		fitnessEvaluator = new CudaFitnessEvaluator(
			args->sourceWidth,
			args->sourceHeight,
			args->diagramPointsCount,
			args->sourceImageData,
			args->sourceDataRowWidthInBytes);
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
	float fitness = fitnessEvaluator->calculateFitness(diagram);
	onIteration(fitness);
	return fitness;
	//float cudaFitness = fitnessEvaluator->calculateFitness(diagram);
	//float cpuFitness = cpuFitnessEvaluator->calculateFitness(diagram);
	//printf("%f %f %f\n", cudaFitness, cpuFitness, cudaFitness - cpuFitness);
	//onIteration(cudaFitness);
	//return cudaFitness;
}

int CompressorAlgorithm::compress(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int * pixelPointAssignment) {
	if (args->limitByTime) {
		Utils::recordTime(&computationStartTime);
	}
	else {
		fitnessEvaluator->resetFitnessCalculationCount();
	}

	// Open log file
	if (args->logFileName != NULL) {
		errno_t err = fopen_s(
			&logFile,
			args->logFileName,
			"ab");
		if (err != 0 || logFile == NULL) {
			return Compressor::ERROR_FILE_COULD_NOT_OPEN_FILE;
		}
	}

	int result = compressInternal(outputDiagram, colors, pixelPointAssignment);
	if (args->logFileName != NULL) {
		fprintf(logFile, "\n");
		fflush(logFile);
		fclose(logFile);
	}
	return result;
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

void CompressorAlgorithm::onIteration(float fitness) {
	bool isFirstIteration = bestFitness == -1;
	if (isFirstIteration || fitness < bestFitness) {
		bestFitness = fitness;
		if (args->logImprovementToConsole) {
			printf("Found better solution with fitness %f\n", bestFitness);
		}
	}
	if (args->logFileName != NULL) {
		fprintf(logFile, isFirstIteration ? "%f" : ";%f", bestFitness);
	}
}

void CompressorAlgorithm::onBestSolutionFound(float bestFitness) {
	printf("Found best solution with fitness %f\n", bestFitness);
}
