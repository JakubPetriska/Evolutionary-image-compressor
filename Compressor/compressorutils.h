#pragma once

#include "compressoralgorithms.h"

namespace lossycompressor {
	class CompressorUtils {
		static void quicksortDiagramPoints(VoronoiDiagram * diagram,
		int start, int end,
		int(*compare) (VoronoiDiagram *, int, int));
	public:
		static void copyPoint(VoronoiDiagram * source, VoronoiDiagram * destination, int index);
		static void copy(VoronoiDiagram * source, VoronoiDiagram * destination);
		static void generateRandomDiagram(VoronoiDiagram * output,
			int32_t sourceWidth, int32_t sourceHeight,
			int(*compare) (VoronoiDiagram *, int, int));
		static void swap(VoronoiDiagram ** first, VoronoiDiagram ** second);
	};
}