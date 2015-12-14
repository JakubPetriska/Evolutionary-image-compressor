#include "compressorutils.h"

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
