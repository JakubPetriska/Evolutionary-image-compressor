#pragma once

#include <cstdint>
#include <memory>
#include "voronoidiagram.h"
#include "fitnesscalculator.h"
#include "color.h"

#define NOMINMAX
#include "Windows.h"

namespace lossycompressor {

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
		LARGE_INTEGER computationStartTime;
		int fitnessEvaluationCount = 0;

		FitnessCalculator * fitnessCalculator;
	protected:
		CompressorAlgorithmArgs * args;

		/*
			Returns fitness of given diagram. Returned fitness is always
			>= 0 with 0 being the best possible value.
			*/
		float calculateFitness(VoronoiDiagram * diagram);

		static int compare(VoronoiDiagram * diagram, int firstPointIndex, int secondPointIndex);

		static int compare(int32_t firstX, int32_t firstY, int32_t secondX, int32_t secondY);

		bool canContinueComputing();

		void onBetterSolutionFound(float bestFitness);

		void onBestSolutionFound(float bestFitness);

		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int ** pixelPointAssignment) = 0;
	public:
		CompressorAlgorithm(CompressorAlgorithmArgs* args)
			: args(args) {};

		int compress(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int ** pixelPointAssignment);
	};
}