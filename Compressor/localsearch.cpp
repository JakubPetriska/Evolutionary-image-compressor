#include "localsearch.h"
#include "utils.h"
#include <random>

using namespace std;
using namespace lossycompressor;

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

	startComputationTimer();

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

		if (nextFitness < currentFitness) {
			swap(&current, &next);
			currentFitness = nextFitness;
			pointToTweak = nextPointToTweak;
			pointTweakTrialCount = 0;

			printf("Found better solution with fitness %f\n", currentFitness);
		}
		else {
			++pointTweakTrialCount;
		}

		if (!canContinueComputing()) {
			printf("Ending after %.4f seconds of calculation\n", currentComputationTime());
			break;
		}
	}


	printf("Found best solution with fitness %f\n", currentFitness);

	// Copy the coordinates of points from the result diagram we obtained to the output diagram
	copy(current, outputDiagram);
	calculateColors(outputDiagram, colors, pixelPointAssignment);

	delete current;
	delete next;

	return 0;
}