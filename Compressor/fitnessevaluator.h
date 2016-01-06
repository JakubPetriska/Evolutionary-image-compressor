#pragma once

#include "voronoidiagram.h"

namespace lossycompressor {

	class FitnessEvaluator {
		int fitnessEvaluationsCount = 0;
	protected:
		int sourceWidth;
		int sourceHeight;
		int diagramPointsCount;
		uint8_t * sourceImageData;
		int sourceDataRowWidthInBytes;

		virtual float calculateFitnessInternal(VoronoiDiagram * diagram) = 0;

		virtual bool isCuda() = 0;
	public:
		FitnessEvaluator(int sourceWidth, int sourceHeight,
			int diagramPointsCount, 
			uint8_t * sourceImageData, int sourceDataRowWidthInBytes);
		virtual ~FitnessEvaluator();

		float calculateFitness(VoronoiDiagram * diagram);

		int getFitnessEvaluationsCount();

		void resetFitnessCalculationCount();
	};
}