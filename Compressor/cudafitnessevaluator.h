#pragma once

#include "fitnessevaluator.h"
#include "voronoidiagram.h"
#include "color.h"

namespace lossycompressor {
	class CudaFitnessEvaluator : public FitnessEvaluator {
		// All pointers to work variables point to device (GPU) memory

		// Holds sums of colors and counts of pixels
		float * rSums;
		int * rCounts;
		float * gSums;
		int * gCounts;
		float * bSums;
		int * bCounts;

		// Array used to store assignment of color to diagram points
		Color24bit * colors;

		// array used to hold assignments of pixels to diagram points
		int * pixelPointAssignment;

		// Image data on device
		uint8_t * devSourceImageData;
	protected:
		virtual float calculateFitnessInternal(VoronoiDiagram * diagram);
	public:
		CudaFitnessEvaluator(int sourceWidth, int sourceHeight,
			int diagramPointsCount,
			uint8_t * sourceImageData, int sourceDataRowWidthInBytes);

		~CudaFitnessEvaluator();
	};
}