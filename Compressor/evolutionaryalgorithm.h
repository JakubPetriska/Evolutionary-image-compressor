#pragma once

#include "localsearch.h"

namespace lossycompressor {
	/*
	Uses evolutionary algorithm to come up with best position of diagram points.
	*/
	class EvolutionaryAlgorithm : public LocalSearch {
		const int POPULATION_SIZE = 10;

		// Percentage of population that is removed by selection
		const float SELECTION_RATE = 0.5f;
		// Percentage of individuals filled into population by crossover instead of mutation
		const float CROSSOVER_RATE = 0.5f;
	protected:
		void crossover(VoronoiDiagram * firstParent, 
			VoronoiDiagram * secondParent,
			VoronoiDiagram * firstChild,
			VoronoiDiagram * secondChild);
	public:
		EvolutionaryAlgorithm(CompressorAlgorithmArgs* args)
			: LocalSearch(args) {};

		virtual int compress(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int ** pixelPointAssignment) override;
	};
}