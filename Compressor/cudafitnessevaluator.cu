#include "cudafitnessevaluator.h"
#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cstdio>
#include <cstdlib>
#include "compressorutils.h"
#include "utils.h"

using namespace std;
using namespace lossycompressor;

#define BLOCK_SIZE 32

static void HandleError(cudaError_t error, const char *file, int line) {
	if (error != cudaSuccess) {
		printf("%s in %s at line %d\n", cudaGetErrorString(error), file, line);
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

	CHECK_ERROR(cudaMalloc((void**)&colors, diagramPointsCount*sizeof(Color24bit)));
	CHECK_ERROR(cudaMalloc((void**)&pixelPointAssignment, sourceHeight*sourceWidth*sizeof(int)));

	int sourceDataSize = sourceHeight*sourceWidth * 3 * sizeof(uint8_t);
	CHECK_ERROR(cudaMalloc((void**)&devSourceImageData, sourceDataSize));
	CHECK_ERROR(cudaMemcpy(devSourceImageData, sourceImageData, sourceDataSize, cudaMemcpyHostToDevice));

	// Allocate arrays for voronoi diagram saved on device
	diagramPointsCoordinatesSize = diagramPointsCount * sizeof(int32_t);
	int32_t * devDiagramPointsXCoordinates;
	CHECK_ERROR(cudaMalloc((void**)&devDiagramPointsXCoordinates, diagramPointsCoordinatesSize));
	int32_t * devDiagramPointsYCoordinates;
	CHECK_ERROR(cudaMalloc((void**)&devDiagramPointsYCoordinates, diagramPointsCoordinatesSize));

	diagram = new VoronoiDiagram(diagramPointsCount, devDiagramPointsXCoordinates, devDiagramPointsYCoordinates);
	CHECK_ERROR(cudaMalloc((void**)&devDiagram, sizeof(VoronoiDiagram)));
	CHECK_ERROR(cudaMemcpy(devDiagram, diagram, sizeof(VoronoiDiagram), cudaMemcpyHostToDevice));

	// TODO release all these
}

CudaFitnessEvaluator::~CudaFitnessEvaluator() {}

__device__ int compare(int firstX, int firstY, int secondX, int secondY) {
	if (firstX == secondX && firstY == secondY) {
		return 0;
	}
	else if (firstX < secondX
		|| (firstX == secondX && firstY < secondY)) {
		return -1;
	}
	else {
		return 1;
	}
}

__device__ int32_t x(VoronoiDiagram * diagram, int index) {
	return diagram->diagramPointsXCoordinates[index];
}

__device__ int32_t y(VoronoiDiagram * diagram, int index) {
	return diagram->diagramPointsYCoordinates[index];
}

__device__ double calculateSquareDistance(int firstX, int firstY, int secondX, int secondY) {
	return ((firstX - secondX) * (firstX - secondX)) + ((firstY - secondY) * (firstY - secondY));
}

__device__ int findClosestHorizontalPoint(int diagramPointsCount, VoronoiDiagram * diagram, int pixelX, int pixelY) {
	if (diagramPointsCount == 1) {
		return 0;
	}

	int start = 0, end = diagramPointsCount;
	while (start < end - 2) {
		int pivotIndex = (start + end) / 2;
		int pixelPivotComparison = compare(pixelX, pixelY, x(diagram, pivotIndex), y(diagram, pivotIndex));
		if (pixelPivotComparison == 0) {
			return pivotIndex;
		}
		else if (pixelPivotComparison < 0) {
			end = pivotIndex + 1;
		}
		else {
			start = pivotIndex;
		}
	}

	double startPixelSquareDist = calculateSquareDistance(pixelX, pixelY, x(diagram, start), y(diagram, start));
	double endPixelSquareDist = calculateSquareDistance(pixelX, pixelY, x(diagram, end), y(diagram, end));
	if (startPixelSquareDist < endPixelSquareDist) {
		return start;
	}
	else {
		return end;
	}
}

__device__ int calculateDiagramPointIndexForPixel(int diagramPointsCount, VoronoiDiagram * diagram,
	int pixelXCoord, int pixelYCoord) {

	int startIndex = findClosestHorizontalPoint(diagramPointsCount, diagram, pixelXCoord, pixelYCoord);
	int currentClosestPointIndex = startIndex;
	double squareDistanceToClosest = calculateSquareDistance(
		x(diagram, currentClosestPointIndex), y(diagram, currentClosestPointIndex),
		pixelXCoord, pixelYCoord);

	bool unacceptableLowerFound = false;
	bool unacceptableHigherFound = false;

	bool lower = false;
	for (int i = 1; i <= diagramPointsCount; i = lower ? i : i + 1) {
		if (unacceptableLowerFound && unacceptableHigherFound) {
			break;
		}

		lower = !lower;
		if ((lower && unacceptableLowerFound)
			|| (!lower && unacceptableHigherFound)) {
			continue;
		}

		int currentIndex = lower ? startIndex - i : startIndex + i;
		if (currentIndex < 0 || currentIndex >= diagramPointsCount) {
			if (lower) {
				unacceptableLowerFound = true;
			}
			else {
				unacceptableHigherFound = true;
			}
			continue;
		}

		double squareDistanceToCurrent = calculateSquareDistance(
			x(diagram, currentIndex), y(diagram, currentIndex),
			pixelXCoord, pixelYCoord);

		if (squareDistanceToCurrent < squareDistanceToClosest) {
			currentClosestPointIndex = currentIndex;
			squareDistanceToClosest = squareDistanceToCurrent;
			unacceptableLowerFound = false;
			unacceptableHigherFound = false;
		}
		else if (lower && !unacceptableLowerFound && x(diagram, currentIndex) < pixelXCoord
			&& ((x(diagram, currentIndex) - pixelXCoord) * (x(diagram, currentIndex) - pixelXCoord)) > squareDistanceToClosest) {
			unacceptableLowerFound = true;
		}
		else if (!lower && !unacceptableHigherFound && x(diagram, currentIndex) > pixelXCoord
			&& ((x(diagram, currentIndex) - pixelXCoord) * (x(diagram, currentIndex) - pixelXCoord)) > squareDistanceToClosest) {
			unacceptableHigherFound = true;
		}
	}

	return currentClosestPointIndex;
}

__global__ void resetWorkVarsKernel(
	int diagramPointsCount,
	int sourceWidth,
	int sourceHeight,
	float * rSums,
	int * rCounts,
	float * gSums,
	int * gCounts,
	float * bSums,
	int * bCounts) {

	int index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index < diagramPointsCount) {
		rSums[index] = 0;
		rCounts[index] = 0;
		gSums[index] = 0;
		gCounts[index] = 0;
		bSums[index] = 0;
		bCounts[index] = 0;
	}
}

__global__ void calculateColorsSumsKernel(
	VoronoiDiagram * devDiagram,
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
	int * pixelPointAssignment) {

	int pixelHorizontal = blockIdx.x * blockDim.x + threadIdx.x;
	int pixelVertical = blockIdx.y * blockDim.y + threadIdx.y;

	// If pixel of this thread is in the image
	if (pixelHorizontal < sourceWidth && pixelVertical < sourceHeight) {
		int linearIndex = pixelHorizontal + sourceWidth * pixelVertical;
		int colorStartIndexInSourceData = pixelHorizontal * 3 + pixelVertical * sourceDataRowWidthInBytes;

		// Find diagram points for all pixels and calculate colors of individual points
		int pointIndex = calculateDiagramPointIndexForPixel(diagramPointsCount, devDiagram, pixelHorizontal, pixelVertical);

		pixelPointAssignment[linearIndex] = pointIndex;

		atomicAdd(bSums + pointIndex, devSourceImageData[colorStartIndexInSourceData]);
		atomicAdd(bCounts + pointIndex, 1);
		atomicAdd(gSums + pointIndex, devSourceImageData[colorStartIndexInSourceData + 1]);
		atomicAdd(gCounts + pointIndex, 1);
		atomicAdd(rSums + pointIndex, devSourceImageData[colorStartIndexInSourceData + 2]);
		atomicAdd(rCounts + pointIndex, 1);
	}
}

__global__ void calculateColorsKernel(
	int diagramPointsCount,
	int sourceWidth,
	int sourceHeight,
	float * rSums,
	int * rCounts,
	float * gSums,
	int * gCounts,
	float * bSums,
	int * bCounts,
	Color24bit * colors) {

	int index = threadIdx.x + blockIdx.x * blockDim.x;
	if (index < diagramPointsCount) {
		Color24bit * color = &colors[index];
		color->b = (uint8_t)(bSums[index] / bCounts[index] + 0.5);
		color->g = (uint8_t)(gSums[index] / gCounts[index] + 0.5);
		color->r = (uint8_t)(rSums[index] / rCounts[index] + 0.5);
	}
}

__global__ void calculateFitnessKernel(
	float * outputFitness,
	int sourceWidth,
	int sourceHeight,
	uint8_t * devSourceImageData,
	int sourceDataRowWidthInBytes,
	Color24bit * colors,
	int * pixelPointAssignment) {

	int pixelHorizontal = blockIdx.x * blockDim.x + threadIdx.x;
	int pixelVertical = blockIdx.y * blockDim.y + threadIdx.y;

	// If pixel of this thread is in images
	if (pixelHorizontal < sourceWidth && pixelVertical < sourceHeight) {
		int linearIndex = pixelHorizontal + pixelVertical * sourceWidth;
		int colorStartIndexInSourceData = pixelVertical * sourceDataRowWidthInBytes + pixelHorizontal * 3;

		int pointIndex = pixelPointAssignment[linearIndex];
		Color24bit color = colors[pointIndex];

		float pixelDeviation
			= (fabsf((float)(devSourceImageData[colorStartIndexInSourceData] - color.b)) // Absolute red color deviation
			+ fabsf((float)(devSourceImageData[colorStartIndexInSourceData + 1] - color.g)) // Absolute green color deviation
			+ fabsf((float)(devSourceImageData[colorStartIndexInSourceData + 2] - color.r))); // Absolute blue color deviation

		atomicAdd(outputFitness, pixelDeviation);
	}
}

float CudaFitnessEvaluator::calculateFitnessInternal(VoronoiDiagram * diagram) {
	float * devFitness;
	CHECK_ERROR(cudaMalloc((void**)&devFitness, sizeof(float)));
	CHECK_ERROR(cudaMemset((void*)devFitness, 0, sizeof(float)));

	CHECK_ERROR(cudaMemcpy(this->diagram->diagramPointsXCoordinates, diagram->diagramPointsXCoordinates,
		diagramPointsCoordinatesSize, cudaMemcpyHostToDevice));
	CHECK_ERROR(cudaMemcpy(this->diagram->diagramPointsYCoordinates, diagram->diagramPointsYCoordinates,
		diagramPointsCoordinatesSize, cudaMemcpyHostToDevice));

	int everyPointThreadCount = BLOCK_SIZE * BLOCK_SIZE;
	int everyPointBlocksCount = diagramPointsCount / everyPointThreadCount;
	if (diagramPointsCount - everyPointBlocksCount * everyPointThreadCount > 0) {
		++everyPointBlocksCount;
	}

	int gridWidth = sourceWidth / BLOCK_SIZE;
	if (sourceWidth - gridWidth * BLOCK_SIZE > 0) {
		++gridWidth;
	}
	int gridHeight = sourceHeight / BLOCK_SIZE;
	if (sourceHeight - gridHeight * BLOCK_SIZE > 0) {
		++gridHeight;
	}

	dim3 everyPixelBlocks(gridWidth, gridHeight);
	dim3 everyPixelThreads(BLOCK_SIZE, BLOCK_SIZE);

	resetWorkVarsKernel << <everyPointBlocksCount, everyPointThreadCount >> >(
		diagramPointsCount,
		sourceWidth, sourceHeight,
		rSums, rCounts, gSums, gCounts, bSums, bCounts);

	calculateColorsSumsKernel << <everyPixelBlocks, everyPixelThreads >> >(
		devDiagram, diagramPointsCount,
		sourceWidth, sourceHeight,
		devSourceImageData, sourceDataRowWidthInBytes,
		rSums, rCounts, gSums, gCounts, bSums, bCounts,
		pixelPointAssignment);

	calculateColorsKernel << <everyPointBlocksCount, everyPointThreadCount >> >(
		diagramPointsCount,
		sourceWidth, sourceHeight,
		rSums, rCounts, gSums, gCounts, bSums, bCounts,
		colors);

	calculateFitnessKernel << <everyPixelBlocks, everyPixelThreads >> >(
		devFitness,
		sourceWidth, sourceHeight,
		devSourceImageData,
		sourceDataRowWidthInBytes,
		colors, pixelPointAssignment);

	// Copy back result fitness
	float fitness = 0;
	CHECK_ERROR(cudaMemcpy(&fitness, devFitness, sizeof(float), cudaMemcpyDeviceToHost));
	return fitness / (sourceWidth * sourceHeight);
}