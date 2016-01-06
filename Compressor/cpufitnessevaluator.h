#pragma once

#include "fitnessevaluator.h"
#include "voronoidiagram.h"
#include "color.h"

namespace lossycompressor {

	/// Calculates fitness using only CPU.
	class CpuFitnessEvaluator : public FitnessEvaluator {
		float * rSums;
		float * gSums;
		float * bSums;
		int * pixelPerPointCounts;

		// Array used to store assignment of color to diagram points
		Color24bit * colorsTmp;

		// 2 dimensional array used to hold assignments of pixels to diagram points
		int * pixelPointAssignment;
		
		/*
		Does binary search for closet value in the sorted diagram points.

		Returns index of such point.
		*/
		int findClosestHorizontalPoint(VoronoiDiagram * diagram, int pixelX, int pixelY);

		int calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
			int pixelXCoord, int pixelYCoord);

	protected:
		virtual float calculateFitnessInternal(VoronoiDiagram * diagram);

		virtual bool isCuda();
	public:
		CpuFitnessEvaluator(int sourceWidth, int sourceHeight, 
			int diagramPointsCount, 
			uint8_t * sourceImageData, int sourceDataRowWidthInBytes);

		~CpuFitnessEvaluator();

		/// Calculates average colors of all points in diagram into the colors array.
		void calculateColors(VoronoiDiagram * diagram,
			Color24bit * colors,
			int * pixelPointAssignment);
	};
}