#include "compressorutils.h"
#include "utils.h"
#include <random>

using namespace std;
using namespace lossycompressor;

void CompressorUtils::copyPoint(VoronoiDiagram * source, VoronoiDiagram * destination, int index) {
	destination->diagramPointsXCoordinates[index] = source->diagramPointsXCoordinates[index];
	destination->diagramPointsYCoordinates[index] = source->diagramPointsYCoordinates[index];
}

void CompressorUtils::copy(VoronoiDiagram * source, VoronoiDiagram * destination) {
	for (int i = 0; i < source->diagramPointsCount; ++i) {
		copyPoint(source, destination, i);
	}
}

void CompressorUtils::generateRandomDiagram(VoronoiDiagram * output,
	int32_t sourceWidth, int32_t sourceHeight, int(*compare) (VoronoiDiagram *, int, int)) {

	random_device rd;
	float widthMultiplier = ((float)(sourceWidth - 1)) / rd.max();
	float heightMultiplier = ((float)(sourceHeight - 1)) / rd.max();

	for (int i = 0; i < output->diagramPointsCount; ++i) {
		output->diagramPointsXCoordinates[i] = (int32_t)(rd() * widthMultiplier + 0.5f);
		output->diagramPointsYCoordinates[i] = (int32_t)(rd() * heightMultiplier + 0.5f);
	}

	quicksortDiagramPoints(output, 0, output->diagramPointsCount, compare);
}

void CompressorUtils::quicksortDiagramPoints(
	VoronoiDiagram * diagram, int start, int end,
	int(*compare) (VoronoiDiagram *, int, int)) {

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

		quicksortDiagramPoints(diagram, start, upperHalfStart, compare);
		quicksortDiagramPoints(diagram, upperHalfStart + 1, end, compare);
	}
}

void CompressorUtils::swap(VoronoiDiagram ** first, VoronoiDiagram ** second) {
	VoronoiDiagram * tmp = *first;
	*first = *second;
	*second = tmp;
}