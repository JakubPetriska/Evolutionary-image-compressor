#include "compressor.h"
#include "compressoralgorithms.h"
#include "utils.h"
#include <cmath>
#include <assert.h>

using namespace std;
using namespace lossycompressor;

int32_t VoronoiDiagram::x(int index) {
	return diagramPointsXCoordinates[index];
}

int32_t VoronoiDiagram::y(int index) {
	return diagramPointsYCoordinates[index];
}

float CompressorAlgorithm::calculateFitness(VoronoiDiagram * diagram) {
	if (args->useCuda) {
		return calculateFitnessCuda(diagram);
	}
	else {
		return calculateFitnessCpu(diagram);
	}
}

float CompressorAlgorithm::calculateFitnessCpu(VoronoiDiagram * diagram) {
	++fitnessEvaluationCount;

	//LARGE_INTEGER startTime, endTime;
	//Utils::recordTime(&startTime);

	calculateColors(diagram, colorsTmp, pixelPointAssignment);
	float fitness = 0;

	for (int i = 0; i < args->sourceHeight; ++i) {
		uint8_t * row = args->sourceImageData[i];
		for (int j = 0; j < args->sourceWidth; ++j) {
			int pointIndex = pixelPointAssignment[i][j];
			Color24bit color = colorsTmp[pointIndex];
			int colorStartIndexInSourceData = j * 3;

			float pixelDeviation
				= (abs((float)(row[colorStartIndexInSourceData] - color.b)) // Absolute red color deviation
				+ abs((float)(row[colorStartIndexInSourceData + 1] - color.g)) // Absolute green color deviation
				+ abs((float)(row[colorStartIndexInSourceData + 2] - color.r))) // Absolute blue color deviation
				/ 255.0f;

			fitness += pixelDeviation;
		}
	}

	//Utils::recordTime(&endTime);
	//double calculationTotalTime = Utils::calculateInterval(&startTime, &endTime);
	//printf("Calculating fitness took %.4f seconds\n", calculationTotalTime);
	return fitness;
}

void CompressorAlgorithm::calculateColors(VoronoiDiagram * diagram,
	Color24bit * colors,
	int ** pixelPointAssignment) {

	for (int i = 0; i < args->diagramPointsCount; ++i) {
		rSums[i] = 0;
		rCounts[i] = 0;
		gSums[i] = 0;
		gCounts[i] = 0;
		bSums[i] = 0;
		bCounts[i] = 0;
	}

	for (int i = 0; i < args->sourceHeight; ++i) {
		uint8_t * row = args->sourceImageData[i];
		for (int j = 0; j < args->sourceWidth; ++j) {
			int pointIndex = calculateDiagramPointIndexForPixel(diagram, j, i);
			assert(pointIndex >= 0);
			pixelPointAssignment[i][j] = pointIndex;

			int colorStartIndexInSourceData = j * 3;
			bSums[pointIndex] += row[colorStartIndexInSourceData];
			bCounts[pointIndex] += 1;
			gSums[pointIndex] += row[colorStartIndexInSourceData + 1];
			gCounts[pointIndex] += 1;
			rSums[pointIndex] += row[colorStartIndexInSourceData + 2];
			rCounts[pointIndex] += 1;
		}
	}

	for (int i = 0; i < args->diagramPointsCount; ++i) {
		Color24bit * color = &colors[i];
		color->b = (uint8_t)(bSums[i] / bCounts[i] + 0.5);
		color->g = (uint8_t)(gSums[i] / gCounts[i] + 0.5);
		color->r = (uint8_t)(rSums[i] / rCounts[i] + 0.5);
	}
}

int CompressorAlgorithm::calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
	int pixelXCoord, int pixelYCoord) {

	int startIndex = findClosestHorizontalPoint(diagram, pixelXCoord, pixelYCoord);
	int currentClosestPointIndex = startIndex;
	double squareDistanceToClosest = Utils::calculateSquareDistance(
		diagram->x(currentClosestPointIndex), diagram->y(currentClosestPointIndex),
		pixelXCoord, pixelYCoord);
	bool unacceptableLowerFound = false;
	bool unacceptableHigherFound = false;

	bool lower = false;
	for (int i = 1; i <= args->diagramPointsCount; i = lower ? i : i + 1) {
		if (unacceptableLowerFound && unacceptableHigherFound) {
			break;
		}

		lower = !lower;
		if ((lower && unacceptableLowerFound)
			|| (!lower && unacceptableHigherFound)) {
			continue;
		}

		int currentIndex = lower ? startIndex - i : startIndex + i;
		if (currentIndex < 0 || currentIndex >= args->diagramPointsCount) {
			if (lower) {
				unacceptableLowerFound = true;
			}
			else {
				unacceptableHigherFound = true;
			}
			continue;
		}

		double squareDistanceToCurrent = Utils::calculateSquareDistance(
			diagram->x(currentIndex), diagram->y(currentIndex),
			pixelXCoord, pixelYCoord);

		if (squareDistanceToCurrent < squareDistanceToClosest) {
			currentClosestPointIndex = currentIndex;
			squareDistanceToClosest = squareDistanceToCurrent;
			unacceptableLowerFound = false;
			unacceptableHigherFound = false;
		}
		else if (lower && !unacceptableLowerFound && diagram->x(currentIndex) < pixelXCoord
			&& pow(diagram->x(currentIndex) - pixelXCoord, 2) > squareDistanceToClosest) {
			unacceptableLowerFound = true;
		}
		else if (!lower && !unacceptableHigherFound && diagram->x(currentIndex) > pixelXCoord
			&& pow(diagram->x(currentIndex) - pixelXCoord, 2) > squareDistanceToClosest) {
			unacceptableHigherFound = true;
		}
	}

	return currentClosestPointIndex;
}

int CompressorAlgorithm::findClosestHorizontalPoint(VoronoiDiagram * diagram, int32_t pixelX, int32_t pixelY) {
	if (args->diagramPointsCount == 1) {
		return 0;
	}

	int start = 0, end = args->diagramPointsCount;
	while (start < end - 2) {
		int pivotIndex = (start + end) / 2;
		int pixelPivotComparison = compare(pixelX, pixelY, diagram->x(pivotIndex), diagram->y(pivotIndex));
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

	assert(start == end - 2);

	double startPixelSquareDist = Utils::calculateSquareDistance(pixelX, pixelY, diagram->x(start), diagram->y(start));
	double endPixelSquareDist = Utils::calculateSquareDistance(pixelX, pixelY, diagram->x(end), diagram->y(end));
	if (startPixelSquareDist < endPixelSquareDist) {
		return start;
	}
	else {
		return end;
	}
}

float CompressorAlgorithm::calculateFitnessCuda(VoronoiDiagram * diagram) {

	return 0;
}

int CompressorAlgorithm::compare(VoronoiDiagram * diagram, int firstPointIndex, int secondPointIndex) {
	return compare(diagram->x(firstPointIndex), diagram->y(firstPointIndex),
		diagram->x(secondPointIndex), diagram->y(secondPointIndex));
}

int CompressorAlgorithm::compare(int32_t firstX, int32_t firstY, int32_t secondX, int32_t secondY) {
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

int CompressorAlgorithm::compress(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {
	if (args->limitByTime) {
		Utils::recordTime(&computationStartTime);
	}
	else {
		fitnessEvaluationCount = 0;
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
		return fitnessEvaluationCount < args->maxFitnessEvaluationCount;
	}
}

void CompressorAlgorithm::onBetterSolutionFound(float bestFitness) {
	printf("Found better solution with fitness %f\n", bestFitness);
}

void CompressorAlgorithm::onBestSolutionFound(float bestFitness) {
	printf("Found best solution with fitness %f\n", bestFitness);
}
