#pragma once

#include "evolutionaryalgorithm.h"

namespace lossycompressor {
	/// Uses memetic algorithm to come up with best position of diagram points.
	class MemeticAlgorithm : public EvolutionaryAlgorithm {
		const int POPULATION_SIZE = 10;

		// Percentage of population that is removed by selection
		const float SELECTION_RATE = 0.5f;
		// Percentage of individuals filled into population by crossover instead of mutation
		const float CROSSOVER_RATE = 0.5f;

		const int LOCAL_SEARCH_ITERATIONS = 30;
	protected:
		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int * pixelPointAssignment) override;
	public:
		MemeticAlgorithm(CompressorAlgorithm::Args* args)
			: EvolutionaryAlgorithm(args) {};
	};
}