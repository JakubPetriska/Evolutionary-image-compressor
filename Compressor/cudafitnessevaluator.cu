#include "cudafitnessevaluator.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cstdio>
#include <cstdlib>

using namespace std;
using namespace lossycompressor;

#define BLOCK_SIZE 32

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
	CHECK_ERROR(cudaMalloc((void**)&pixelPointAssignment, sourceHeight*sourceWidth*sizeof(int)));

	int sourceDataSize = sourceHeight*sourceWidth * 3 * sizeof(uint8_t);
	CHECK_ERROR(cudaMalloc((void**)&devSourceImageData, sourceDataSize));
	CHECK_ERROR(cudaMemcpy(devSourceImageData, sourceImageData, sourceDataSize, cudaMemcpyHostToDevice));
}

CudaFitnessEvaluator::~CudaFitnessEvaluator() {}

//__device__ int calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
//	int pixelXCoord, int pixelYCoord) {
//
//	int startIndex = findClosestHorizontalPoint(diagram, pixelXCoord, pixelYCoord);
//	int currentClosestPointIndex = startIndex;
//	double squareDistanceToClosest = Utils::calculateSquareDistance(
//		diagram->x(currentClosestPointIndex), diagram->y(currentClosestPointIndex),
//		pixelXCoord, pixelYCoord);
//	bool unacceptableLowerFound = false;
//	bool unacceptableHigherFound = false;
//
//	bool lower = false;
//	for (int i = 1; i <= diagramPointsCount; i = lower ? i : i + 1) {
//		if (unacceptableLowerFound && unacceptableHigherFound) {
//			break;
//		}
//
//		lower = !lower;
//		if ((lower && unacceptableLowerFound)
//			|| (!lower && unacceptableHigherFound)) {
//			continue;
//		}
//
//		int currentIndex = lower ? startIndex - i : startIndex + i;
//		if (currentIndex < 0 || currentIndex >= diagramPointsCount) {
//			if (lower) {
//				unacceptableLowerFound = true;
//			}
//			else {
//				unacceptableHigherFound = true;
//			}
//			continue;
//		}
//
//		double squareDistanceToCurrent = Utils::calculateSquareDistance(
//			diagram->x(currentIndex), diagram->y(currentIndex),
//			pixelXCoord, pixelYCoord);
//
//		if (squareDistanceToCurrent < squareDistanceToClosest) {
//			currentClosestPointIndex = currentIndex;
//			squareDistanceToClosest = squareDistanceToCurrent;
//			unacceptableLowerFound = false;
//			unacceptableHigherFound = false;
//		}
//		else if (lower && !unacceptableLowerFound && diagram->x(currentIndex) < pixelXCoord
//			&& pow(diagram->x(currentIndex) - pixelXCoord, 2) > squareDistanceToClosest) {
//			unacceptableLowerFound = true;
//		}
//		else if (!lower && !unacceptableHigherFound && diagram->x(currentIndex) > pixelXCoord
//			&& pow(diagram->x(currentIndex) - pixelXCoord, 2) > squareDistanceToClosest) {
//			unacceptableHigherFound = true;
//		}
//	}
//
//	return currentClosestPointIndex;
//}
//
//__device__ int findClosestHorizontalPoint(VoronoiDiagram * diagram, int32_t pixelX, int32_t pixelY) {
//	if (diagramPointsCount == 1) {
//		return 0;
//	}
//
//	int start = 0, end = diagramPointsCount;
//	while (start < end - 2) {
//		int pivotIndex = (start + end) / 2;
//		int pixelPivotComparison = CompressorUtils::compare(pixelX, pixelY, diagram->x(pivotIndex), diagram->y(pivotIndex));
//		if (pixelPivotComparison == 0) {
//			return pivotIndex;
//		}
//		else if (pixelPivotComparison < 0) {
//			end = pivotIndex + 1;
//		}
//		else {
//			start = pivotIndex;
//		}
//	}
//
//	assert(start == end - 2);
//
//	double startPixelSquareDist = Utils::calculateSquareDistance(pixelX, pixelY, diagram->x(start), diagram->y(start));
//	double endPixelSquareDist = Utils::calculateSquareDistance(pixelX, pixelY, diagram->x(end), diagram->y(end));
//	if (startPixelSquareDist < endPixelSquareDist) {
//		return start;
//	}
//	else {
//		return end;
//	}
//}

__global__ void fitnessKernel(
	VoronoiDiagram * devDiagram,
	float * outputFitness,
	int diagramPointsCount,
	int sourceWidth,
	int sourceHeight,
	uint8_t * devSourceImageData,
	int sourceDataRowWidthInBytes,
	float * rSums,
	int * rCounts,
	float * gSums,
	int * gCounts,
	float * bSums,
	int * bCounts,
	Color24bit * colorsTmp,
	int * pixelPointAssignment) {

	int pixelHorizontal = blockIdx.x * blockDim.x + threadIdx.x;
	int pixelVertical = blockIdx.y * blockDim.y + threadIdx.y;
	int linearIndex = pixelHorizontal * sourceWidth + pixelVertical;

	// If pixel of this thread is in images
	if (pixelHorizontal < sourceWidth && pixelVertical < sourceHeight) {
		// Reset the work variables
		if (linearIndex < diagramPointsCount) {
			rSums[linearIndex] = 0;
			rCounts[linearIndex] = 0;
			gSums[linearIndex] = 0;
			gCounts[linearIndex] = 0;
			bSums[linearIndex] = 0;
			bCounts[linearIndex] = 0;
		}

		// Find diagram points for all pixels and calculate colors of individual points
		//int pointIndex = calculateDiagramPointIndexForPixel(diagram, j, i);
		int pointIndex = 1;
		
		pixelPointAssignment[linearIndex] = pointIndex;

		int colorStartIndexInSourceData = pixelVertical * sourceDataRowWidthInBytes + pixelHorizontal * 3;
		bSums[pointIndex] += devSourceImageData[colorStartIndexInSourceData];
		bCounts[pointIndex] += 1;
		gSums[pointIndex] += devSourceImageData[colorStartIndexInSourceData + 1];
		gCounts[pointIndex] += 1;
		rSums[pointIndex] += devSourceImageData[colorStartIndexInSourceData + 2];
		rCounts[pointIndex] += 1;


		// Calculate the fitness of this pixel
		Color24bit color = colorsTmp[pointIndex];

		float pixelDeviation
			= (fabsf((float)(devSourceImageData[colorStartIndexInSourceData] - color.b)) // Absolute red color deviation
			+ fabsf((float)(devSourceImageData[colorStartIndexInSourceData + 1] - color.g)) // Absolute green color deviation
			+ fabsf((float)(devSourceImageData[colorStartIndexInSourceData + 2] - color.r))) // Absolute blue color deviation
			/ 255.0f;

		atomicAdd(outputFitness, pixelDeviation);
	}
}

float CudaFitnessEvaluator::calculateFitnessInternal(VoronoiDiagram * diagram) {
	float * devFitness;
	CHECK_ERROR(cudaMalloc((void**)&devFitness, sizeof(float)));
	VoronoiDiagram * devDiagram;
	CHECK_ERROR(cudaMalloc((void**)&devDiagram, sizeof(VoronoiDiagram)));
	CHECK_ERROR(cudaMemcpy(devDiagram, diagram, sizeof(VoronoiDiagram), cudaMemcpyHostToDevice));

	int gridWidth = sourceWidth / BLOCK_SIZE;
	if (sourceWidth - gridWidth * BLOCK_SIZE > 0) {
		++gridWidth;
	}
	int gridHeight = sourceHeight / BLOCK_SIZE;
	if (sourceHeight - gridHeight * BLOCK_SIZE > 0) {
		++gridHeight;
	}

	dim3 blocks(gridWidth, gridHeight);
	dim3 threads(BLOCK_SIZE, BLOCK_SIZE);

	fitnessKernel << <blocks, threads >> >(
		devDiagram, devFitness,
		diagramPointsCount, sourceWidth, sourceHeight,
		devSourceImageData, sourceDataRowWidthInBytes,
		rSums, rCounts, gSums, gCounts, bSums, bCounts,
		colorsTmp, pixelPointAssignment);

	// Copy back result fitness
	float fitness;
	CHECK_ERROR(cudaMemcpy(&fitness, devFitness, sizeof(float), cudaMemcpyDeviceToHost));

	CHECK_ERROR(cudaFree(devDiagram));
	return fitness;
}