#include "voronoidiagram.h"

using namespace lossycompressor;

int32_t VoronoiDiagram::x(int index) {
	return diagramPointsXCoordinates[index];
}

int32_t VoronoiDiagram::y(int index) {
	return diagramPointsYCoordinates[index];
}