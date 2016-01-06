#include "fitnessevaluator.h"
#include <cstdio>
#include "utils.h"

using namespace lossycompressor;

FitnessEvaluator::FitnessEvaluator(
	int sourceWidth, int sourceHeight, 
	int diagramPointsCount, 
	uint8_t * sourceImageData, int sourceDataRowWidthInBytes)
	: sourceWidth(sourceWidth),
	sourceHeight(sourceHeight),
	diagramPointsCount(diagramPointsCount),
	sourceImageData(sourceImageData),
	sourceDataRowWidthInBytes(sourceDataRowWidthInBytes) {}

FitnessEvaluator::~FitnessEvaluator() {}

double fitnessCalculationLengthsSum = 0;

float FitnessEvaluator::calculateFitness(VoronoiDiagram * diagram) {
	LARGE_INTEGER startTime, endTime;
	Utils::recordTime(&startTime);
	
	++fitnessEvaluationsCount;
	float fitness = calculateFitnessInternal(diagram);

	Utils::recordTime(&endTime);
	double calculationTotalTime = Utils::calculateInterval(&startTime, &endTime);

	fitnessCalculationLengthsSum += calculationTotalTime;
	std::printf("Calculating fitness took %.4f seconds, average time is %.4f\n", 
		calculationTotalTime, fitnessCalculationLengthsSum / fitnessEvaluationsCount);

	return fitness;
}

int FitnessEvaluator::getFitnessEvaluationsCount() {
	return fitnessEvaluationsCount;
}

void FitnessEvaluator::resetFitnessCalculationCount() {
	fitnessEvaluationsCount = 0;
}