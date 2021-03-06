#include "cpufitnessevaluator.h"
#include "compressorutils.h"
#include "utils.h"
#include <cmath>
#include <assert.h>
#include <cstdio>

using namespace std;
using namespace lossycompressor;

CpuFitnessEvaluator::CpuFitnessEvaluator(
	int sourceWidth, int sourceHeight,
	int diagramPointsCount, 
	uint8_t * sourceImageData, int sourceDataRowWidthInBytes)
	: FitnessEvaluator(sourceWidth, sourceHeight, diagramPointsCount, sourceImageData, sourceDataRowWidthInBytes),
	rSums(new float[diagramPointsCount]),
	gSums(new float[diagramPointsCount]),
	bSums(new float[diagramPointsCount]),
	pixelPerPointCounts(new int[diagramPointsCount]),
	colorsTmp(new Color24bit[diagramPointsCount]),
	pixelPointAssignment(new int[sourceHeight * sourceWidth]) {};

CpuFitnessEvaluator::~CpuFitnessEvaluator() {
	delete[] rSums;
	delete[] gSums;
	delete[] bSums;
	delete[] colorsTmp;
	delete[] pixelPointAssignment;
}

float CpuFitnessEvaluator::calculateFitnessInternal(VoronoiDiagram * diagram) {
	calculateColors(diagram, colorsTmp, pixelPointAssignment);

	float fitness = 0;
	for (int i = 0; i < sourceHeight; ++i) {
		for (int j = 0; j < sourceWidth; ++j) {
			int pointIndex = pixelPointAssignment[i * sourceWidth + j];
			Color24bit color = colorsTmp[pointIndex];
			int colorStartIndexInSourceData = i * sourceDataRowWidthInBytes + j * 3;

			float pixelDeviation
				= (abs((float)(sourceImageData[colorStartIndexInSourceData] - color.b)) // Absolute red color deviation
				+ abs((float)(sourceImageData[colorStartIndexInSourceData + 1] - color.g)) // Absolute green color deviation
				+ abs((float)(sourceImageData[colorStartIndexInSourceData + 2] - color.r))); // Absolute blue color deviation

			fitness += pixelDeviation;
		}
	}
	return fitness / (sourceWidth * sourceHeight);
}

void CpuFitnessEvaluator::calculateColors(VoronoiDiagram * diagram,
	Color24bit * colors,
	int * pixelPointAssignment) {

	for (int i = 0; i < diagramPointsCount; ++i) {
		rSums[i] = 0;
		gSums[i] = 0;
		bSums[i] = 0;
		pixelPerPointCounts[i] = 0;
	}

	for (int i = 0; i < sourceWidth; ++i) {
		for (int j = 0; j < sourceHeight; ++j) {
			int pointIndex = calculateDiagramPointIndexForPixel(diagram, i, j);
			assert(pointIndex >= 0);
			pixelPointAssignment[i + j * sourceWidth] = pointIndex;

			int colorStartIndexInSourceData = i * 3 + j * sourceDataRowWidthInBytes;
			bSums[pointIndex] += sourceImageData[colorStartIndexInSourceData];
			gSums[pointIndex] += sourceImageData[colorStartIndexInSourceData + 1];
			rSums[pointIndex] += sourceImageData[colorStartIndexInSourceData + 2];
			pixelPerPointCounts[pointIndex] += 1;
		}
	}

	for (int i = 0; i < diagramPointsCount; ++i) {
		Color24bit * color = &colors[i];
		color->b = (uint8_t)(bSums[i] / pixelPerPointCounts[i] + 0.5);
		color->g = (uint8_t)(gSums[i] / pixelPerPointCounts[i] + 0.5);
		color->r = (uint8_t)(rSums[i] / pixelPerPointCounts[i] + 0.5);
	}
}

int CpuFitnessEvaluator::calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
	int pixelXCoord, int pixelYCoord) {

	int startIndex = findClosestHorizontalPoint(diagram, pixelXCoord, pixelYCoord);
	int currentClosestPointIndex = startIndex;
	double squareDistanceToClosest = Utils::calculateSquareDistance(
		diagram->x(currentClosestPointIndex), diagram->y(currentClosestPointIndex),
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

int CpuFitnessEvaluator::findClosestHorizontalPoint(VoronoiDiagram * diagram, int pixelX, int pixelY) {
	if (diagramPointsCount == 1) {
		return 0;
	}

	int start = 0, end = diagramPointsCount;
	while (start < end - 2) {
		int pivotIndex = (start + end) / 2;
		int pixelPivotComparison = CompressorUtils::compare(pixelX, pixelY, diagram->x(pivotIndex), diagram->y(pivotIndex));
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

bool CpuFitnessEvaluator::isCuda() {
	return false;
}