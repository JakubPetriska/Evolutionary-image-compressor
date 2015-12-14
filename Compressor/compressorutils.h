#pragma once

#include "voronoidiagram.h"

namespace lossycompressor {
	class CompressorUtils {
		static void quicksortDiagramPoints(VoronoiDiagram * diagram, int start, int end);
	public:
		static void copyPoint(VoronoiDiagram * source, VoronoiDiagram * destination, int index);

		static void copy(VoronoiDiagram * source, VoronoiDiagram * destination);

		static void generateRandomDiagram(VoronoiDiagram * output,
			int32_t sourceWidth, int32_t sourceHeight);

		static void swap(VoronoiDiagram ** first, VoronoiDiagram ** second);

		/*
		Returns the same values as compare(int32_t, int32_t, int32_t, int32_t).
		*/
		static int compare(VoronoiDiagram * diagram, int firstPointIndex, int secondPointIndex);

		/*
		Returns number < 0 if first point x coordinate is smaller
		than second point's x coordinate or x coordinates are equal
		and first point's y coordinate is smaller than second point's
		y coordinate. Returns 0 if points are equal, otherwise returns
		number > 0.
		*/
		static int compare(int32_t firstX, int32_t firstY, int32_t secondX, int32_t secondY);
	};
}