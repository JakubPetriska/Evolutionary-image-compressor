#pragma once

#include "voronoidiagram.h"

namespace lossycompressor {

	/// Contains utility methods used in compression.
	class CompressorUtils {
		static void quicksortDiagramPoints(VoronoiDiagram * diagram, int start, int end);
	public:
		/// Copy point on given index from source diagram into the destionation diagram.
		static void copyPoint(VoronoiDiagram * source, VoronoiDiagram * destination, int index);

		/// Copy the source diagram into the destionation diagram.
		static void copy(VoronoiDiagram * source, VoronoiDiagram * destination);

		/// Generates random diagram.
		static void generateRandomDiagram(VoronoiDiagram * output,
			int32_t sourceWidth, int32_t sourceHeight);

		/// Swap two diagrams on given pointers.
		static void swap(VoronoiDiagram ** first, VoronoiDiagram ** second);

		/// Returns the same values as compare(int32_t, int32_t, int32_t, int32_t).
		static int compare(VoronoiDiagram * diagram, int firstPointIndex, int secondPointIndex);

		/// Compare two points on X axis.
		/**
			Returns number < 0 if first point x coordinate is smaller
			than second point's x coordinate or x coordinates are equal
			and first point's y coordinate is smaller than second point's
			y coordinate. Returns 0 if points are equal, otherwise returns
			number > 0.
		*/
		static int compare(int firstX, int firstY, int secondX, int secondY);
	};
}