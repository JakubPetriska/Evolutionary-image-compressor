#pragma once

#include "compressoralgorithms.h"

namespace lossycompressor {
	class CompressorUtils {
	public:
		static void copyPoint(VoronoiDiagram * source, VoronoiDiagram * destination, int index);
		static void copy(VoronoiDiagram * source, VoronoiDiagram * destination);
	};
}