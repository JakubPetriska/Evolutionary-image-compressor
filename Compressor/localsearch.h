#pragma once

#include "compressoralgorithm.h"

namespace lossycompressor {
	/// Uses hill-climbing to come up with best position of diagram points.
	class LocalSearch : public CompressorAlgorithm {
		const int MAX_POINT_TO_TWEAK_TRIAL_COUNT = 10;
	protected:
		/// Tweaks the source diagram and copies it into destination diagram.
		void tweak(VoronoiDiagram * source, VoronoiDiagram * destination);
	public:
		LocalSearch(CompressorAlgorithm::Args* args)
			: CompressorAlgorithm(args) {};

	protected:
		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int * pixelPointAssignment) override;
	};
}