#include "localsearch.h"
#include "compressorutils.h"
#include "utils.h"
#include <random>

using namespace std;
using namespace lossycompressor;

void LocalSearch::tweak(VoronoiDiagram * source, VoronoiDiagram * destination) {
	// TODO do this adaptive, also bit mor reasonable

	float movementPerc = 0.3f;

	random_device rd;
	int pointToTweak = (int)(rd() * (((float)(args->diagramPointsCount - 1)) / rd.max()) + 0.5f);

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

int LocalSearch::compressInternal(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {

	VoronoiDiagram * current = new VoronoiDiagram(args->diagramPointsCount);
	float currentFitness = -1;

	VoronoiDiagram * next = new VoronoiDiagram(args->diagramPointsCount);
	float nextFitness = -1;

	// Generate random diagram as our starting position
	CompressorUtils::generateRandomDiagram(current, args->sourceWidth, args->sourceHeight, compare);
	currentFitness = calculateFitness(current);

	// Try few random diagrams - it's possible to generate pretty good staring point just randomly
	for (int i = 0; i < 15 && canContinueComputing(); ++i) {
		CompressorUtils::generateRandomDiagram(next, args->sourceWidth, args->sourceHeight, compare);
		nextFitness = calculateFitness(next);
		if (nextFitness < currentFitness) {
			CompressorUtils::swap(&current, &next);
			currentFitness = nextFitness;
		}
	}

	while (canContinueComputing()) {
		tweak(current, next);
		nextFitness = calculateFitness(next);

		if (nextFitness < currentFitness) {
			CompressorUtils::swap(&current, &next);
			currentFitness = nextFitness;

			onBetterSolutionFound(currentFitness);
		}
	}

	onBestSolutionFound(currentFitness);

	// Copy the coordinates of points from the result diagram we obtained to the output diagram
	CompressorUtils::copy(current, outputDiagram);
	calculateColors(outputDiagram, colors, pixelPointAssignment);

	delete current;
	delete next;

	return 0;
}