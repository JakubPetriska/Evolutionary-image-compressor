#include "memeticalgorithm.h"
#include "compressorutils.h"

using namespace std;
using namespace lossycompressor;

int MemeticAlgorithm::compressInternal(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {

	int selectionSize = POPULATION_SIZE * SELECTION_RATE;
	int breedingSize = selectionSize * CROSSOVER_RATE;
	int mutationSize = selectionSize - breedingSize;

	vector<VoronoiDiagram*> diagramPool;
	diagramPool.push_back(new VoronoiDiagram(args->diagramPointsCount));

	vector<VoronoiDiagram*> population;
	vector<float> populationFitness;

	VoronoiDiagram * best = NULL;
	float bestFitness;

	generateInitialPopulation(POPULATION_SIZE,
		&population, &populationFitness,
		&bestFitness, &best);

	onBetterSolutionFound(bestFitness);

	while (canContinueComputing()) {
		selection(selectionSize, &population, &populationFitness, &diagramPool);
		
		// Improve selected individuals by local search
		for (int i = 0; i < population.size() && canContinueComputing(); ++i) {
			VoronoiDiagram * member = population[i];
			float memberFitness = populationFitness[i];
			population.erase(population.begin() + i);
			populationFitness.erase(populationFitness.begin() + i);

			VoronoiDiagram * freeMember = diagramPool.back();
			diagramPool.pop_back();

			for (int j = 0; j < LOCAL_SEARCH_ITERATIONS && canContinueComputing(); ++j) {
				tweak(member, freeMember);
				float tweakedMemberFitness = calculateFitness(freeMember);
				if (tweakedMemberFitness < memberFitness) {
					memberFitness = tweakedMemberFitness;
					VoronoiDiagram * tmp = member;
					member = freeMember;
					freeMember = tmp;

					if (memberFitness < bestFitness) {
						bestFitness = memberFitness;
						best = member;

						onBetterSolutionFound(bestFitness);
					}
				}
			}

			diagramPool.push_back(freeMember);
			population.insert(population.begin() + i, member);
			populationFitness.insert(populationFitness.begin() + i, memberFitness);
		}
		
		int selectedPopSize = population.size();
		breeding(breedingSize, selectedPopSize - 1,
			&population, &populationFitness, &diagramPool,
			&bestFitness, &best);

		mutation(mutationSize, selectedPopSize - 1,
			&population, &populationFitness, &diagramPool,
			&bestFitness, &best);
	}

	onBestSolutionFound(bestFitness);

	// Copy the coordinates of points from the result diagram we obtained to the output diagram
	CompressorUtils::copy(best, outputDiagram);
	calculateColors(outputDiagram, colors, pixelPointAssignment);

	for (int i = 0; i < population.size(); ++i) {
		delete population[i];
	}

	for (int i = 0; i < diagramPool.size(); ++i) {
		delete diagramPool[i];
	}

	return 0;
}
