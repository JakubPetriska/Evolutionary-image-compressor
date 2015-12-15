#pragma once

#include "fitnessevaluator.h"
#include "voronoidiagram.h"
#include "color.h"

namespace lossycompressor {
	class CudaFitnessEvaluator : public FitnessEvaluator {

	protected:
		virtual float calculateFitnessInternal(VoronoiDiagram * diagram);
	public:
		CudaFitnessEvaluator(int sourceWidth, int sourceHeight,
			int diagramPointsCount, uint8_t ** sourceImageData);

		~CudaFitnessEvaluator();
	};
}