#include "cudafitnessevaluator.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace lossycompressor;

static void HandleError(cudaError_t error, const char *file, int line) {
	if (error != cudaSuccess) {
		printf("%s in %s at line %d\n", cudaGetErrorString(error), file, line);
		scanf(" ");
		exit(EXIT_FAILURE);
	}
}

#define CHECK_ERROR( error ) ( HandleError( error, __FILE__, __LINE__ ) )

CudaFitnessEvaluator::CudaFitnessEvaluator(
	int sourceWidth, int sourceHeight,
	int diagramPointsCount, 
	uint8_t * sourceImageData, int sourceDataRowWidthInBytes)
	: FitnessEvaluator(sourceWidth, sourceHeight, diagramPointsCount, sourceImageData, sourceDataRowWidthInBytes) {

	CHECK_ERROR(cudaMalloc((void**)&rSums, diagramPointsCount*sizeof(float)));
	CHECK_ERROR(cudaMalloc((void**)&rCounts, diagramPointsCount*sizeof(int)));
	CHECK_ERROR(cudaMalloc((void**)&gSums, diagramPointsCount*sizeof(float)));
	CHECK_ERROR(cudaMalloc((void**)&gCounts, diagramPointsCount*sizeof(int)));
	CHECK_ERROR(cudaMalloc((void**)&bSums, diagramPointsCount*sizeof(float)));
	CHECK_ERROR(cudaMalloc((void**)&bCounts, diagramPointsCount*sizeof(int)));
	
	CHECK_ERROR(cudaMalloc((void**)&colorsTmp, diagramPointsCount*sizeof(Color24bit)));
	CHECK_ERROR(cudaMalloc((void**)&pixelPointAssignment, sourceHeight*sizeof(int*)));
	// TODO Maybe make pixel point assignment one dimensional
}

CudaFitnessEvaluator::~CudaFitnessEvaluator() {}



float CudaFitnessEvaluator::calculateFitnessInternal(VoronoiDiagram * diagram) {
	float * devFitness;
	CHECK_ERROR(cudaMalloc((void**)&devFitness, sizeof(float)));
	VoronoiDiagram * devDiagram;
	CHECK_ERROR(cudaMalloc((void**)&devDiagram, sizeof(VoronoiDiagram)));
	CHECK_ERROR(cudaMemcpy(devDiagram, diagram, sizeof(VoronoiDiagram), cudaMemcpyHostToDevice));



	// Copy back result fitness
	float fitness;
	CHECK_ERROR(cudaMemcpy(&fitness, devFitness, sizeof(float), cudaMemcpyDeviceToHost));

	CHECK_ERROR(cudaFree(devDiagram));
	return fitness;
}