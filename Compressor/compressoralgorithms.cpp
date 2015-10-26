#include "compressor.h"
#include "compressoralgorithms.h"
#include <cmath>
#include <random>
#include "utils.h"
#include <assert.h>

using namespace std;
using namespace lossycompressor;

int32_t VoronoiDiagram::x(int index) {
	return diagramPointsXCoordinates[index];
}

int32_t VoronoiDiagram::y(int index) {
	return diagramPointsYCoordinates[index];
}

int CompressorAlgorithm::calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
	int pixelXCoord, int pixelYCoord) {

	int startIndex = findClosestHorizontalPoint(diagram, pixelXCoord, pixelYCoord);
	int currentClosestPointIndex = startIndex;
	double squareDistanceToClosest = calculateSquareDistance(
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

		double squareDistanceToCurrent = calculateSquareDistance(
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

	double startPixelSquareDist = calculateSquareDistance(pixelX, pixelY, diagram->x(start), diagram->y(start));
	double endPixelSquareDist = calculateSquareDistance(pixelX, pixelY, diagram->x(end), diagram->y(end));
	if (startPixelSquareDist < endPixelSquareDist) {
		return start;
	}
	else {
		return end;
	}
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

// TODO is this way of calculation of fitness ok? Maybe simmulated annealing should benefit from something else.
float CompressorAlgorithm::calculateFitness(VoronoiDiagram * diagram, int * worstDeviationPerPixelPointIndex) {
	__int64 startTime, endTime;
	Utils::getCurrentMillis(&startTime);

	calculateColors(diagram, colorsTmp, pixelPointAssignment);
	float fitness = 0;

	if (worstDeviationPerPixelPointIndex != NULL) {
		for (int i = 0; i < args->diagramPointsCount; ++i) {
			deviationSumPerPoint[i] = 0;
			pixelCountPerPoint[i] = 0;
		}
	}

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

			if (worstDeviationPerPixelPointIndex != NULL) {
				deviationSumPerPoint[pointIndex] += pixelDeviation;
				pixelCountPerPoint[pointIndex] += 1;
			}
		}
	}

	if (worstDeviationPerPixelPointIndex != NULL) {
		float minDeviationPerPixel;
		float currentMinDeviationPerPixelPointIndex = -1;
		for (int i = 0; i < args->diagramPointsCount; ++i) {
			float deviationPerPixel = deviationSumPerPoint[i] / pixelCountPerPoint[i];
			if (currentMinDeviationPerPixelPointIndex == -1 
				|| deviationPerPixel < minDeviationPerPixel) {
				minDeviationPerPixel = deviationPerPixel;
				currentMinDeviationPerPixelPointIndex = i;
			}
		}
		*worstDeviationPerPixelPointIndex = currentMinDeviationPerPixelPointIndex;
	}

	Utils::getCurrentMillis(&endTime);
	double calculationTotalTime = (endTime - startTime) / 1000.0;
	printf("Calculating fitness took %.4f seconds\n", calculationTotalTime);
	return fitness;
}

void CompressorAlgorithm::generateRandomDiagram(VoronoiDiagram * output) {
	random_device rd;
	float widthMultiplier = ((float)(args->sourceWidth - 1)) / rd.max();
	float heightMultiplier = ((float)(args->sourceHeight - 1)) / rd.max();

	for (int i = 0; i < args->diagramPointsCount; ++i) {
		output->diagramPointsXCoordinates[i] = (int32_t)(rd() * widthMultiplier + 0.5f);
		output->diagramPointsYCoordinates[i] = (int32_t)(rd() * heightMultiplier + 0.5f);
	}

	quicksortDiagramPoints(output);
}

double CompressorAlgorithm::calculateSquareDistance(int32_t firstX, int32_t firstY, int32_t secondX, int32_t secondY) {
	return pow(firstX - secondX, 2) + pow(firstY - secondY, 2);
}

void CompressorAlgorithm::quicksortDiagramPoints(
	VoronoiDiagram * diagram, int start, int end) {

	if (end == -1) {
		end = args->diagramPointsCount;
	}

	if (start < end - 1) {
		int pivotIndex = end - 1;
		int upperHalfStart = start;
		for (int i = start; i < pivotIndex; ++i) {
			if (compare(diagram, i, pivotIndex) == -1) {
				Utils::swap(diagram->diagramPointsXCoordinates, i, upperHalfStart);
				Utils::swap(diagram->diagramPointsYCoordinates, i, upperHalfStart);
				++upperHalfStart;
			}
		}
		Utils::swap(diagram->diagramPointsXCoordinates, pivotIndex, upperHalfStart);
		Utils::swap(diagram->diagramPointsYCoordinates, pivotIndex, upperHalfStart);

		quicksortDiagramPoints(diagram, start, upperHalfStart);
		quicksortDiagramPoints(diagram, upperHalfStart + 1, end);
	}
}

void CompressorAlgorithm::swap(VoronoiDiagram ** first, VoronoiDiagram ** second) {
	VoronoiDiagram * tmp = *first;
	*first = *second;
	*second = tmp;
}

void CompressorAlgorithm::copy(VoronoiDiagram * source, VoronoiDiagram * destination) {
	for (int i = 0; i < args->diagramPointsCount; ++i) {
		destination->diagramPointsXCoordinates[i] = source->diagramPointsXCoordinates[i];
		destination->diagramPointsYCoordinates[i] = source->diagramPointsYCoordinates[i];
	}
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

void LocalSearch::tweak(VoronoiDiagram * source, VoronoiDiagram * destination, int pointToTweak) {
	// TODO do this adaptive, also bit mor reasonable

	float movementPerc = 0.3f;

	random_device rd;
	if (pointToTweak == -1) {
		pointToTweak = (int)(rd() * (((float)(args->diagramPointsCount - 1)) / rd.max()) + 0.5f);
	}

	float halfRdMax = rd.max() / 2;
	float horizontalMovementMultiplier = ((float)args->sourceWidth) / halfRdMax;
	float verticalMovementMultiplier = ((float)args->sourceHeight) / halfRdMax;

	int32_t xDelta = (int32_t)((rd() - halfRdMax) * horizontalMovementMultiplier * movementPerc);
	int32_t yDelta = (int32_t)((rd() - halfRdMax) * verticalMovementMultiplier * movementPerc);

	for (int i = 0; i < args->diagramPointsCount; ++i) {
		destination->diagramPointsXCoordinates[i] = source->diagramPointsXCoordinates[i];
		destination->diagramPointsYCoordinates[i] = source->diagramPointsYCoordinates[i];
		if (i == pointToTweak) {
			destination->diagramPointsXCoordinates[i] += xDelta;
			destination->diagramPointsYCoordinates[i] += yDelta;
		}
	}

	// Maintain the sorted order of diagram points
	int currentIndex = pointToTweak;
	if (xDelta > 0 || (xDelta == 0 && yDelta > 0)) {
		while (currentIndex < args->diagramPointsCount - 1 && compare(destination, currentIndex, currentIndex + 1) == 1) {
			Utils::swap(destination->diagramPointsXCoordinates, currentIndex, currentIndex + 1);
			Utils::swap(destination->diagramPointsYCoordinates, currentIndex, currentIndex + 1);
			++currentIndex;
		}
	}
	else if (xDelta < 0 || (xDelta == 0 && yDelta < 0)) {
		while (currentIndex > 0 && compare(destination, currentIndex - 1, currentIndex) == 1) {
			Utils::swap(destination->diagramPointsXCoordinates, currentIndex, currentIndex - 1);
			Utils::swap(destination->diagramPointsYCoordinates, currentIndex, currentIndex - 1);
			--currentIndex;
		}
	}
}

int LocalSearch::compress(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {

	__int64 startTime, currentTime;
	Utils::getCurrentMillis(&startTime);

	VoronoiDiagram * current = new VoronoiDiagram(args->diagramPointsCount);
	float currentFitness = -1;

	VoronoiDiagram * next = new VoronoiDiagram(args->diagramPointsCount);
	float nextFitness = -1;

	int pointTweakTrialCount = 0;
	int pointToTweak = 0;
	int nextPointToTweak = 0;

	// Generate random diagram as our starting position
	generateRandomDiagram(current);
	currentFitness = calculateFitness(current, &pointToTweak);

	// Try few random diagrams - it's possible to generate pretty good staring point just randomly
	for (int i = 0; i < 15; ++i) {
		generateRandomDiagram(next);
		nextFitness = calculateFitness(next, &nextPointToTweak);
		if (nextFitness < currentFitness) {
			swap(&current, &next);
			currentFitness = nextFitness;
			pointToTweak = nextPointToTweak;
		}
	}

	while (true) {
		tweak(current, next, pointTweakTrialCount < MAX_POINT_TO_TWEAK_TRIAL_COUNT ? pointToTweak : -1);
		nextFitness = calculateFitness(next, &nextPointToTweak);

		if (nextFitness < currentFitness) { // TODO maybe do simulated annealing
			swap(&current, &next);
			currentFitness = nextFitness;
			pointToTweak = nextPointToTweak;
			pointTweakTrialCount = 0;

			printf("Found better solution with fitness %f\n", currentFitness);
		}
		else {
			++pointTweakTrialCount;
		}

		Utils::getCurrentMillis(&currentTime);
		double calculationTimeSecs = (currentTime - startTime) / 1000.0;
		if (calculationTimeSecs >= MAX_CALCULATION_TIME_SECONDS) {
			printf("Ending after %.4f seconds of calculation\n", calculationTimeSecs);
			break;
		}
	}


	printf("Found best solution with fitness %f\n", currentFitness);

	// Copy the coordinates of points from the result diagram we obtained to the output diagram
	copy(current, outputDiagram);
	calculateColors(outputDiagram, colors, pixelPointAssignment);
	return 0;
}