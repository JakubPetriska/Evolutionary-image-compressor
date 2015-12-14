#pragma once

#include "fitnesscalculator.h"
#include "voronoidiagram.h"
#include "color.h"

namespace lossycompressor {
	class CpuFitnessCalculator : FitnessCalculator {
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
	public:

		CpuFitnessCalculator(int sourceWidth, int sourceHeight, int diagramPointsCount)
			:rSums(new float[diagramPointsCount]),
			rCounts(new int[diagramPointsCount]),
			gSums(new float[diagramPointsCount]),
			gCounts(new int[diagramPointsCount]),
			bSums(new float[diagramPointsCount]),
			bCounts(new int[diagramPointsCount]),
			colorsTmp(new Color24bit[diagramPointsCount]),
			pixelPointAssignment(new int*[sourceHeight])			
		{
			for (int i = 0; i < args->sourceHeight; ++i) {
				pixelPointAssignment[i] = new int[sourceWidth];
			}
		};

		~CpuFitnessCalculator() {
			delete[] rSums;
			delete[] rCounts;
			delete[] gSums;
			delete[] gCounts;
			delete[] bSums;
			delete[] bCounts;
			delete[] colorsTmp;
			for (int i = 0; i < args->sourceHeight; ++i) {
				delete[] pixelPointAssignment[i];
			}
			delete[] pixelPointAssignment;
		}

		virtual float calculateFitness(VoronoiDiagram * diagram);

		/*
		Calculates average colors of all points in diagram into the colors array.
		*/
		void calculateColors(VoronoiDiagram * diagram,
			Color24bit * colors,
			int ** pixelPointAssignment);
	};
}