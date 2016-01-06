#pragma once

#include "voronoidiagram.h"

namespace lossycompressor {

	/// Base class for fitness evaluator. Subclasses must provide concrete fitness calculation methods.
	class FitnessEvaluator {
		int fitnessEvaluationsCount = 0;
	protected:
		int sourceWidth;				///< Width of source image.
		int sourceHeight;				///< Height of source image.
		int diagramPointsCount;			///< Count of points in diagram.
		uint8_t * sourceImageData;		///< Data of source image.
		int sourceDataRowWidthInBytes;	///< Length of a row in source image data.

		/// Calculates fitness of given diagram.
		/**
			Subclasses must implement this method to provide their way of fitness ccalculation.
		*/
		virtual float calculateFitnessInternal(VoronoiDiagram * diagram) = 0;

		/// Return true if computation of fitness is accelerated by CUDA.
		virtual bool isCuda() = 0;
	public:
		/// Construct a new FitnessEvaluator.
		FitnessEvaluator(int sourceWidth, int sourceHeight,
			int diagramPointsCount, 
			uint8_t * sourceImageData, int sourceDataRowWidthInBytes);
		virtual ~FitnessEvaluator();

		/// Calculates fitness of given diagram.
		float calculateFitness(VoronoiDiagram * diagram);

		/// Returns count of fitness evaluations done by this evaluator.
		int getFitnessEvaluationsCount();

		/// Resets the fitness evaluations count.
		void resetFitnessCalculationCount();
	};
}