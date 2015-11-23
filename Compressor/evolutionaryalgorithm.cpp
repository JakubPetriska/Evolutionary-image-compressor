#include "evolutionaryalgorithm.h"
#include <vector>

using namespace std;
using namespace lossycompressor;

int EvolutionaryAlgorithm::compress(VoronoiDiagram * outputDiagram,
	Color24bit * colors, int ** pixelPointAssignment) {

	startComputationTimer();

	vector<VoronoiDiagram*> population(POPULATION_SIZE);
	vector<float> populationFitness(POPULATION_SIZE);

	for (int i = 0; i < POPULATION_SIZE; ++i) {
		VoronoiDiagram * populationMember = new VoronoiDiagram(args->diagramPointsCount);
		population.push_back(populationMember);
		generateRandomDiagram(populationMember);
		populationFitness.push_back(calculateFitness(populationMember));
	}

	VoronoiDiagram * best = NULL;

	while (true) {
		if (!canContinueComputing()) {
			printf("Ending after %.4f seconds of calculation\n", currentComputationTime());
			break;
		}
	}

	// Copy the coordinates of points from the result diagram we obtained to the output diagram
	copy(best, outputDiagram);
	calculateColors(outputDiagram, colors, pixelPointAssignment);

	for (int i = 0; i < population.size(); ++i) {
		delete population[i];
	}

	return 0;
}