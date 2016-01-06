#pragma once

#include <cstdint>
#include <memory>
#include "voronoidiagram.h"
#include "cpufitnessevaluator.h"
#include "color.h"

#define NOMINMAX
#include "Windows.h"

namespace lossycompressor {

	/// Class representing an algorithm that compresses the image.
	/**
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
	public:
		/// Instance of this class is passed as a parameter into CompressorAlgorithm. Contains compression input data and compression settings.
		struct Args {
			int32_t sourceWidth;
			int32_t sourceHeight;
			uint8_t * sourceImageData;
			int sourceDataRowWidthInBytes;
			int diagramPointsCount;
			bool limitByTime; // True is algorithm should be limited by time, false if algorithm should be limited by fitness evaluation count
			double maxComputationTimeSecs;
			int maxFitnessEvaluationCount;
			bool useCuda;
			char * logFileName;
			bool logImprovementToConsole;
		};
	private:
		LARGE_INTEGER computationStartTime;

		FitnessEvaluator * fitnessEvaluator;
		
		FILE* logFile;

		float bestFitness = -1;

		void onIteration(float bestFitness);
	protected:
		/// Compression arguments.
		CompressorAlgorithm::Args * args;

		/// Fitness evaluator using CPU.
		CpuFitnessEvaluator * cpuFitnessEvaluator;

		// Calculate fitness of given diagram/
		/**
			Returned fitness is always
			>= 0 with 0 being the best possible value.
			*/
		float calculateFitness(VoronoiDiagram * diagram);

		/**
			\return Returns true if computation can continue given it's limit, false otherwise.
		*/
		bool canContinueComputing();

		/// Must be called when computation found the best solution and will terminate.
		void onBestSolutionFound(float bestFitness);

		/// Implement this method to provide the compression calculation.
		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int * pixelPointAssignment) = 0;
	public:
		/// Construct new CompressionAlgorithm.
		CompressorAlgorithm(CompressorAlgorithm::Args* args);
		~CompressorAlgorithm();

		/// Do the compression.
		/**
			\param[in] outputDiagram			Diagram into which the compression result will be written.
			\param[in] colors					Array into which colors of points in diagram will be written.
			\param[in] pixelPointAssignment		Array into which indices of closest point for every pixel will be written.
		*/
		int compress(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int * pixelPointAssignment);
	};
}