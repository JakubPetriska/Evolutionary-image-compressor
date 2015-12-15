#include "cudafitnessevaluator.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

using namespace lossycompressor;

CudaFitnessEvaluator::CudaFitnessEvaluator(
	int sourceWidth, int sourceHeight,
	int diagramPointsCount, uint8_t ** sourceImageData)
	: FitnessEvaluator(sourceWidth, sourceHeight, diagramPointsCount, sourceImageData) {

}

CudaFitnessEvaluator::~CudaFitnessEvaluator() {}

float CudaFitnessEvaluator::calculateFitnessInternal(VoronoiDiagram * diagram) {
	return 5;
}