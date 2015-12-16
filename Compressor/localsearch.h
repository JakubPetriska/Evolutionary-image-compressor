#pragma once

#include "compressoralgorithm.h"

namespace lossycompressor {
	/*
	Algorithm uses hill-climbing to come up with best position of diagram points.
	*/
	class LocalSearch : public CompressorAlgorithm {
		const int MAX_POINT_TO_TWEAK_TRIAL_COUNT = 10;
	protected:
		/*
		Copies source diagram into destination while tweaking it.
		*/
		void tweak(VoronoiDiagram * source, VoronoiDiagram * destination);
	public:
		LocalSearch(CompressorAlgorithmArgs* args)
			: CompressorAlgorithm(args) {};

	protected:
		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int * pixelPointAssignment) override;
	};
}