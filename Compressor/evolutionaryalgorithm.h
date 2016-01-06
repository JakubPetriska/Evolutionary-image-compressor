#pragma once

#include "localsearch.h"
#include <vector>

using namespace std;

namespace lossycompressor {
	/// Uses evolutionary algorithm to come up with best position of diagram points.
	class EvolutionaryAlgorithm : public LocalSearch {
		const int POPULATION_SIZE = 10;

		// Percentage of population that is removed by selection
		const float SELECTION_RATE = 0.5f;
		// Percentage of individuals filled into population by crossover instead of mutation
		const float CROSSOVER_RATE = 0.5f;
	protected:
		/// Does crossover of given parents and stores the results into given children.
		void crossover(VoronoiDiagram * firstParent,
			VoronoiDiagram * secondParent,
			VoronoiDiagram * firstChild,
			VoronoiDiagram * secondChild);

		/// Generates initial population.
		/**
			/param[in] populationSize	Size of the initial population.
			/param[out] population		Vector into which the generated population will be stored.
			/param[out] populationFitness	Vector in which fitness values of population members will be stored.
			/param[out] bestFitness			On location of this pointer the fitness of best population member will be stored.
			/param[out] best				Into the pointer referenced by this variable best population member will be stored.
		*/
		void generateInitialPopulation(int populationSize,
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			float * bestFitness,
			VoronoiDiagram ** best);

		/// Does selection on given population.
		void selection(int selectionSize, 
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			vector<VoronoiDiagram*> * diagramPool);

		/// Does breeding on given population.
		void breeding(int breedingSize, int maxBredMemberIndex,
			vector<VoronoiDiagram*> * population,
			vector<float> * populationFitness,
			vector<VoronoiDiagram*> * diagramPool,
			float * bestFitness,
			VoronoiDiagram ** best);

		/// Mutates mutationSize new members from current population. Shuffles the population so that same member is not mutated twice.
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
		EvolutionaryAlgorithm(CompressorAlgorithm::Args* args)
			: LocalSearch(args) {};
	};
}