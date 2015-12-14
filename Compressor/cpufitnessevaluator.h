#pragma once

#include "fitnessevaluator.h"
#include "voronoidiagram.h"
#include "color.h"

namespace lossycompressor {
	class CpuFitnessEvaluator : public FitnessEvaluator {
		float * rSums;
		int * rCounts;
		float * gSums;
		int * gCounts;
		float * bSums;
		int * bCounts;

		// Array used to store assignment of color to diagram points
		Color24bit * colorsTmp;

		// 2 dimensional array used to hold assignments of pixels to diagram points
		int ** pixelPointAssignment;
		
		/*
		Does binary search for closet value in the sorted diagram points.

		Returns index of such point.
		*/
		int findClosestHorizontalPoint(VoronoiDiagram * diagram, int32_t pixelX, int32_t pixelY);

		int calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
			int pixelXCoord, int pixelYCoord);

	protected:
		virtual float calculateFitnessInternal(VoronoiDiagram * diagram);
	public:
		CpuFitnessEvaluator(int sourceWidth, int sourceHeight, 
			int diagramPointsCount, uint8_t ** sourceImageData);

		~CpuFitnessEvaluator();

		/*
		Calculates average colors of all points in diagram into the colors array.
		*/
		void calculateColors(VoronoiDiagram * diagram,
			Color24bit * colors,
			int ** pixelPointAssignment);
	};
}