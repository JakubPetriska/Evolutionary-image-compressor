#pragma once

#include "localsearch.h"

namespace lossycompressor {
	/*
	Uses evolutionary algorithm to come up with best position of diagram points.
	*/
	class EvolutionaryAlgorithm : public LocalSearch {
		const int POPULATION_SIZE = 10;
	public:
		EvolutionaryAlgorithm(CompressorAlgorithmArgs* args)
			: LocalSearch(args) {};

		virtual int compress(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int ** pixelPointAssignment) override;
	};
}