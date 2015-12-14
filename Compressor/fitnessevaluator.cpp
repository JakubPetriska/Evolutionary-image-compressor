#include "fitnessevaluator.h"

using namespace lossycompressor;

FitnessEvaluator::FitnessEvaluator(
	int sourceWidth, int sourceHeight, 
	int diagramPointsCount, uint8_t ** sourceImageData)
	: sourceWidth(sourceWidth),
	sourceHeight(sourceHeight),
	diagramPointsCount(diagramPointsCount),
	sourceImageData(sourceImageData) {}

FitnessEvaluator::~FitnessEvaluator() {}

float FitnessEvaluator::calculateFitness(VoronoiDiagram * diagram) {
	++fitnessEvaluationsCount;
	return calculateFitnessInternal(diagram);
}

int FitnessEvaluator::getFitnessEvaluationsCount() {
	return fitnessEvaluationsCount;
}

void FitnessEvaluator::resetFitnessCalculationCount() {
	fitnessEvaluationsCount = 0;
}