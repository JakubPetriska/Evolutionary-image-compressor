#pragma once

#include "localsearch.h"
#include <vector>

using namespace std;

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

		void generateInitialPopulation(int populationSize,
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			float * bestFitness,
			VoronoiDiagram ** best);

		void selection(int selectionSize, 
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			vector<VoronoiDiagram*> * diagramPool);

		void breeding(int breedingSize, int maxBredMemberIndex,
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			vector<VoronoiDiagram*> * diagramPool,
			float * bestFitness,
			VoronoiDiagram ** best);

		/*
			Mutates mutationSize new members from current population.

			Shuffles the population so that same member is not mutated twice.
		*/
		void mutation(int mutationSize, int maxMutatedMemberIndex,
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			vector<VoronoiDiagram*> * diagramPool,
			float * bestFitness,
			VoronoiDiagram ** best);

		virtual int compressInternal(VoronoiDiagram * outputDiagram,
			Color24bit * colors,
			int * pixelPointAssignment) override;
	public:
		EvolutionaryAlgorithm(CompressorAlgorithmArgs* args)
			: LocalSearch(args) {};
	};
}