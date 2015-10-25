#include "compressor.h"
#include "compressoralgorithms.h"
#include <cmath>
#include <random>
#include "utils.h"

using namespace std;
using namespace lossycompressor;

int CompressorAlgorithm::calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
	int pixelXCoord, int pixelYCoord) {

	int closestPointIndex = -1;
	float closestPointSquareDistance = -1;
	for (int i = 0; i < args->diagramPointsCount; ++i) {
		float pointSquareDistance
			= pow((float)diagram->diagramPointsXCoordinates[i] - pixelXCoord, 2)
			+ pow((float)diagram->diagramPointsYCoordinates[i] - pixelYCoord, 2);
		if (closestPointIndex == -1 || pointSquareDistance < closestPointSquareDistance) {
			closestPointIndex = i;
			closestPointSquareDistance = pointSquareDistance;
		}
	}
	return closestPointIndex;
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
float CompressorAlgorithm::calculateFitness(VoronoiDiagram * diagram) {
	__int64 startTime, endTime;
	Utils::getCurrentMillis(&startTime);

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

	Utils::getCurrentMillis(&endTime);
	double calculationTotalTime = (endTime - startTime) / 1000.0;
	//printf("Calculating fitness took %.4f seconds\n", calculationTotalTime);
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

void LocalSearch::tweak(VoronoiDiagram * source, VoronoiDiagram * destination) {
	// TODO do this adaptive

	float movementPerc = 0.1f;

	random_device rd;
	int pointToTweakIndex = (int)(rd() * (((float)(args->diagramPointsCount - 1)) / rd.max()) + 0.5f);

	float halfRdMax = rd.max() / 2;
	float horizontalMovementMultiplier = ((float)args->sourceWidth) / halfRdMax;
	float verticalMovementMultiplier = ((float)args->sourceHeight) / halfRdMax;

	for (int i = 0; i < args->diagramPointsCount; ++i) {
		destination->diagramPointsXCoordinates[i] = source->diagramPointsXCoordinates[i];
		destination->diagramPointsYCoordinates[i] = source->diagramPointsYCoordinates[i];
		if (i == pointToTweakIndex) {
			destination->diagramPointsXCoordinates[i] 
				+= (int32_t)((rd() - halfRdMax) * horizontalMovementMultiplier * movementPerc);

			destination->diagramPointsYCoordinates[i] 
				+= (int32_t)((rd() - halfRdMax) * verticalMovementMultiplier * movementPerc);
		}
	}
}

int LocalSearch::compress(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {

	__int64 startTime, currentTime;
	Utils::getCurrentMillis(&startTime);

	// We'll keep our best result so far in this diagram
	VoronoiDiagram * best = new VoronoiDiagram(args->diagramPointsCount);
	float bestFitness = -1;

	VoronoiDiagram * current = new VoronoiDiagram(args->diagramPointsCount);
	float currentFitness = -1;

	VoronoiDiagram * next = new VoronoiDiagram(args->diagramPointsCount);
	float nextFitness = -1;

	// Generate random diagram as our starting position
	generateRandomDiagram(current);
	currentFitness = calculateFitness(current);

	copy(current, best);
	bestFitness = currentFitness;

	while (true) {
		tweak(current, next);
		nextFitness = calculateFitness(next);

		if (nextFitness < currentFitness) { // TODO maybe do simulated annealing
			swap(&current, &next);
			currentFitness = nextFitness;

			printf("Found better solution with fitness %f\n", currentFitness);
		}

		Utils::getCurrentMillis(&currentTime);
		float calculationTimeSecs = (currentTime - startTime) / 1000.0;
		if (calculationTimeSecs >= MAX_CALCULATION_TIME_SECONDS) {
			printf("Ending after %.4f seconds of calculation\n", calculationTimeSecs);
			break;
		}
	}

	// TODO delete this when we start to keep the best
	copy(current, best);
	bestFitness = currentFitness;


	printf("Found best solution with fitness %f\n", bestFitness);

	// Copy the coordinates of points from the result diagram we obtained to the output diagram
	copy(best, outputDiagram);
	calculateColors(outputDiagram, colors, pixelPointAssignment);
	return 0;
}