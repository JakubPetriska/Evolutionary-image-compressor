#pragma once

#include "fitnessevaluator.h"
#include "voronoidiagram.h"
#include "color.h"

namespace lossycompressor {

	/// Calculates fitness. The calculation is accelerated by CUDA.
	class CudaFitnessEvaluator : public FitnessEvaluator {
		int diagramPointsCoordinatesSize;
		
		// All pointers to work variables point to device (GPU) memory

		// Holds sums of colors and counts of pixels
		float * rSums;
		float * gSums;
		float * bSums;
		int * pixelPerPointCounts;

		// Array used to store assignment of color to diagram points
		Color24bit * colors;

		// array used to hold assignments of pixels to diagram points
		int * pixelPointAssignment;

		// Image data on device
		uint8_t * devSourceImageData;

		VoronoiDiagram * diagram;
		VoronoiDiagram * devDiagram;
	protected:
		virtual float calculateFitnessInternal(VoronoiDiagram * diagram);

		virtual bool isCuda();
	public:
		CudaFitnessEvaluator(int sourceWidth, int sourceHeight,
			int diagramPointsCount,
			uint8_t * sourceImageData, int sourceDataRowWidthInBytes);

		~CudaFitnessEvaluator();
	};
}