#pragma once

#include <cstdint>
#include <memory>

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
	};

	struct CompressorAlgorithmArgs {
		int32_t sourceWidth;
		int32_t sourceHeight;
		uint8_t ** sourceImageData;
		int diagramPointsCount;
	};

	/*
		Class representing an algorithm that compresses the image.

		Implementations use local search or various genetic algorithms.
		Therefore this class contains useful methods used in these implementations.

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

		// Array used to store assignment of color to diagram points
		Color24bit * colorsTmp;

		// 2 dimensional array used to hold assignments of pixels to diagram points
		int ** pixelPointAssignment;

		int calculateDiagramPointIndexForPixel(VoronoiDiagram * diagram, 
			int pixelXCoord, int pixelYCoord);
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

		void copy(VoronoiDiagram * source, VoronoiDiagram * destination);
	public:
		CompressorAlgorithm(CompressorAlgorithmArgs* args)
			: args(args),
			rSums(new float[args->diagramPointsCount]),
			rCounts(new int[args->diagramPointsCount]),
			gSums(new float[args->diagramPointsCount]), 
			gCounts(new int[args->diagramPointsCount]),
			bSums(new float[args->diagramPointsCount]),
			bCounts(new int[args->diagramPointsCount]),
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
			delete[] colorsTmp;
			for (int i = 0; i < args->sourceHeight; ++i) {
				delete[] pixelPointAssignment[i];
			}
			delete[] pixelPointAssignment;
		}

		virtual int compress(VoronoiDiagram * outputDiagram, 
			Color24bit * colors, 
			int ** pixelPointAssignment) = 0;
	};

	class LocalSearch : public CompressorAlgorithm {
		const float MAX_CALCULATION_TIME_SECONDS = 20 * 60;
	public:
		LocalSearch(CompressorAlgorithmArgs* args)
			: CompressorAlgorithm(args) {};

		virtual int compress(VoronoiDiagram * outputDiagram, 
			Color24bit * colors,
			int ** pixelPointAssignment) override;

		/*
			Copies source diagram into destination while tweaking it.
		*/
		void tweak(VoronoiDiagram * source, VoronoiDiagram * destination);
	};
}