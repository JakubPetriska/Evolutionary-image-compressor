#pragma once

#include <cstdint>
#include <memory>

#define NOMINMAX
#include "Windows.h"

namespace lossycompressor {

	struct Color24bit {
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};

	/*
		Represents voronoi diagram.

		Coordinates of diagram points correspond to pixels of the image.
		Coordinates go from bottom left corner of the image (0, 0) to top
		right corner of the image (imageWidth - 1, imageHeight - 1).
		*/
	struct VoronoiDiagram {
		const int32_t diagramPointsCount;
		int32_t * const diagramPointsXCoordinates;
		int32_t * const diagramPointsYCoordinates;

		VoronoiDiagram(int32_t diagramPointsCount)
			: diagramPointsCount(diagramPointsCount),
			diagramPointsXCoordinates(new int32_t[diagramPointsCount]),
			diagramPointsYCoordinates(new int32_t[diagramPointsCount]) {}

		~VoronoiDiagram() {
			delete[] diagramPointsXCoordinates;
			delete[] diagramPointsYCoordinates;
		}

		int32_t x(int index);
		int32_t y(int index);
	};

	struct CompressorAlgorithmArgs {
		int32_t sourceWidth;
		int32_t sourceHeight;
		uint8_t ** sourceImageData;
		int diagramPointsCount;
		bool limitByTime; // True is algorithm should be limited by time, false if algorithm should be limited by fitness evaluation count
		double maxComputationTimeSecs;
		int maxFitnessEvaluationCount;
		bool useCuda;
	};

	/*
		Class representing an algorithm that compresses the image.

		Implementations use local search or various genetic algorithms.
		Therefore this class contains useful methods used in these implementations.

		Points in voronoi diagrams used during calculations are kept sorted
		according to their horizontal (x) coordinate. This sorting speeds
		up fitness calculation. Points are initially
		sorted when diagram is generated and during tweaking only the changed
		point(s) is/are put to their right place.

		BEWARE these utility methods use work variables from the instance of this class
		and hence cannot be executed in parallel.
		*/
	class CompressorAlgorithm {
		// Variables used during calculation of colors
		float * rSums;
		int * rCounts;
		float * gSums;
		int * gCounts;
		float * bSums;
		int * bCounts;

		// Used during calculation of fitness - estimating best point to tweak
		float * deviationSumPerPoint;
		int * pixelCountPerPoint;

		// Array used to store assignment of color to diagram points
		Color24bit * colorsTmp;

		// 2 dimensional array used to hold assignments of pixels to diagram points
		int ** pixelPointAssignment;

		int calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram,
			int pixelXCoord, int pixelYCoord);

		LARGE_INTEGER computationStartTime;
		int fitnessEvaluationCount = 0;

		float calculateFitnessCpu(VoronoiDiagram * diagram);

		float calculateFitnessCuda(VoronoiDiagram * diagram);
	protected:
		CompressorAlgorithmArgs * args;

		/*
			Calculates average colors of all points in diagram into the colors array.
			*/
		void calculateColors(VoronoiDiagram * diagram,
			Color24bit * colors,
			int ** pixelPointAssignment);

		/*
			Returns fitness of given diagram. Returned fitness is always
			>= 0 with 0 being the best possible value.
			*/
		float calculateFitness(VoronoiDiagram * diagram);

		void generateRandomDiagram(VoronoiDiagram * output);

		void swap(VoronoiDiagram ** first, VoronoiDiagram ** second);

		int compare(VoronoiDiagram * diagram, int firstPointIndex, int secondPointIndex);

		int compare(int32_t firstX, int32_t firstY, int32_t secondX, int32_t secondY);

		void quicksortDiagramPoints(VoronoiDiagram * diagram, int start = 0, int end = -1);

		/*
			Does binary search for closet value in the sorted diagram points.

			Returns index of such point.
			*/
		int findClosestHorizontalPoint(VoronoiDiagram * diagram, int32_t pixelX, int32_t pixelY);

		void startComputationTimer();

		bool canContinueComputing();

		void onBetterSolutionFound(float bestFitness);

		void onBestSolutionFound(float bestFitness);

		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int ** pixelPointAssignment) = 0;
	public:
		CompressorAlgorithm(CompressorAlgorithmArgs* args)
			: args(args),
			rSums(new float[args->diagramPointsCount]),
			rCounts(new int[args->diagramPointsCount]),
			gSums(new float[args->diagramPointsCount]),
			gCounts(new int[args->diagramPointsCount]),
			bSums(new float[args->diagramPointsCount]),
			bCounts(new int[args->diagramPointsCount]),
			deviationSumPerPoint(new float[args->diagramPointsCount]),
			pixelCountPerPoint(new int[args->diagramPointsCount]),
			colorsTmp(new Color24bit[args->diagramPointsCount]),
			pixelPointAssignment(new int*[args->sourceHeight])
		{
			for (int i = 0; i < args->sourceHeight; ++i) {
				pixelPointAssignment[i] = new int[args->sourceWidth];
			}
		};
		~CompressorAlgorithm() {
			delete[] rSums;
			delete[] rCounts;
			delete[] gSums;
			delete[] gCounts;
			delete[] bSums;
			delete[] bCounts;
			delete[] deviationSumPerPoint;
			delete[] pixelCountPerPoint;
			delete[] colorsTmp;
			for (int i = 0; i < args->sourceHeight; ++i) {
				delete[] pixelPointAssignment[i];
			}
			delete[] pixelPointAssignment;
		}

		int compress(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int ** pixelPointAssignment);
	};
}