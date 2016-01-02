#pragma once

#include <cstdint>

namespace lossycompressor {
	/*
	Represents voronoi diagram.

	Coordinates of diagram points correspond to pixels of the image.
	Coordinates go from bottom left corner of the image (0, 0) to top
	right corner of the image (imageWidth - 1, imageHeight - 1).
	*/
	struct VoronoiDiagram{
		const int32_t diagramPointsCount;
		int32_t * diagramPointsXCoordinates;
		int32_t * diagramPointsYCoordinates;

		VoronoiDiagram(int32_t diagramPointsCount)
			: diagramPointsCount(diagramPointsCount),
			diagramPointsXCoordinates(new int32_t[diagramPointsCount]),
			diagramPointsYCoordinates(new int32_t[diagramPointsCount]) {}

		VoronoiDiagram(int32_t diagramPointsCount, 
			int32_t * diagramPointsXCoordinates, int32_t * diagramPointsYCoordinates)
			: diagramPointsCount(diagramPointsCount),
			diagramPointsXCoordinates(diagramPointsXCoordinates),
			diagramPointsYCoordinates(diagramPointsYCoordinates) {}

		~VoronoiDiagram() {
			// TODO do something
			//delete[] diagramPointsXCoordinates;
			//delete[] diagramPointsYCoordinates;
		}

		int32_t x(int index);
		int32_t y(int index);
	};
}
