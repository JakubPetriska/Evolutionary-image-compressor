#include "evolutionaryalgorithm.h"
#include "compressorutils.h"
#include "utils.h"
#include <random>

using namespace std;
using namespace lossycompressor;

void EvolutionaryAlgorithm::generateInitialPopulation(int populationSize,
	vector<VoronoiDiagram*> * population,
	vector<float> * populationFitness,
	float * bestFitness,
	VoronoiDiagram ** best) {

	for (int i = 0; i < populationSize && canContinueComputing(); ++i) {
		VoronoiDiagram * populationMember = new VoronoiDiagram(args->diagramPointsCount);
		population->push_back(populationMember);
		CompressorUtils::generateRandomDiagram(populationMember, args->sourceWidth, args->sourceHeight);

		float memberFitness = calculateFitness(populationMember);
		populationFitness->push_back(memberFitness);

		if (*best == NULL || memberFitness < *bestFitness) {
			*best = populationMember;
			*bestFitness = memberFitness;
		}
	}
}

void EvolutionaryAlgorithm::selection(int selectionSize,
	vector<VoronoiDiagram*> * population,
	vector<float> * populationFitness,
	vector<VoronoiDiagram*> * diagramPool) {

	for (int i = 0; i < selectionSize; ++i) {
		// Do tournament selection until we have selected enough
		int firstSelectedIndex = Utils::generateRandom(population->size() - 1);
		int secondSelectedIndex = Utils::generateRandom(population->size() - 1);
		while (secondSelectedIndex == firstSelectedIndex) {
			secondSelectedIndex = Utils::generateRandom(population->size() - 1);
		}

		float firstFitness = (*populationFitness)[firstSelectedIndex];
		float secondFitness = (*populationFitness)[secondSelectedIndex];

		int memberToRemoveIndex = firstFitness > secondFitness ? firstSelectedIndex : secondSelectedIndex;
		diagramPool->push_back((*population)[memberToRemoveIndex]);
		population->erase(population->begin() + memberToRemoveIndex);
		populationFitness->erase(populationFitness->begin() + memberToRemoveIndex);
	}
}

void EvolutionaryAlgorithm::breeding(int breedingSize, int maxBredMemberIndex,
	vector<VoronoiDiagram*> * population,
	vector<float> * populationFitness,
	vector<VoronoiDiagram*> * diagramPool,
	float * bestFitness,
	VoronoiDiagram ** best) {

	for (int i = 0; i < breedingSize && canContinueComputing(); ++i) {
		bool addedNewMember = false;
		while (!addedNewMember && canContinueComputing()) {
			int firstParentIndex = Utils::generateRandom(maxBredMemberIndex);
			int secondParentIndex = Utils::generateRandom(maxBredMemberIndex);

			float firstParentFitness = (*populationFitness)[firstParentIndex];
			float secondParentFitness = (*populationFitness)[secondParentIndex];

			VoronoiDiagram * firstChild = diagramPool->back();
			diagramPool->pop_back();
			VoronoiDiagram * secondChild = diagramPool->back();
			diagramPool->pop_back();

			VoronoiDiagram * newMember;
			float newMemberFitness = -1;

			crossover((*population)[firstParentIndex], (*population)[secondParentIndex],
				firstChild, secondChild);

			float firstChildFitness = calculateFitness(firstChild);
			float secondChildFitness = calculateFitness(secondChild);

			if (firstChildFitness < secondChildFitness) {
				newMember = firstChild;
				newMemberFitness = firstChildFitness;
				diagramPool->push_back(secondChild);
			}
			else {
				newMember = secondChild;
				newMemberFitness = secondChildFitness;
				diagramPool->push_back(firstChild);
			}

			population->push_back(newMember);
			populationFitness->push_back(newMemberFitness);
			addedNewMember = true;

			if (newMemberFitness < *bestFitness) {
				*bestFitness = newMemberFitness;
				*best = newMember;

				onBetterSolutionFound(*bestFitness);
			}
		}
	}
}

void EvolutionaryAlgorithm::mutation(int mutationSize, 
	int maxMutatedMemberIndex,
	vector<VoronoiDiagram*> * population,
	vector<float> * populationFitness,
	vector<VoronoiDiagram*> * diagramPool,
	float * bestFitness,
	VoronoiDiagram ** best) {

	for (int i = 0; i < mutationSize && canContinueComputing(); ++i) {
		int memberToMutateIndex = Utils::generateRandom(maxMutatedMemberIndex);
		VoronoiDiagram * memberToMutate = (*population)[memberToMutateIndex];
		float memberToMutateFitness = (*populationFitness)[memberToMutateIndex];
		population->erase(population->begin() + memberToMutateIndex);
		populationFitness->erase(populationFitness->begin() + memberToMutateIndex);

		VoronoiDiagram * newMember = diagramPool->back();
		diagramPool->pop_back();

		float newMemberFitness = -1;

		tweak(memberToMutate, newMember);
		newMemberFitness = calculateFitness(newMember);

		population->push_back(newMember);
		populationFitness->push_back(newMemberFitness);

		if (newMemberFitness < *bestFitness) {
			*bestFitness = newMemberFitness;
			*best = newMember;

			onBetterSolutionFound(*bestFitness);
		}

		// Put mutated member after elements available for mutation as given by mutationPopSize
		population->insert(population->begin() + maxMutatedMemberIndex + 1, memberToMutate);
		populationFitness->insert(populationFitness->begin() + maxMutatedMemberIndex + 1, memberToMutateFitness);

		--maxMutatedMemberIndex;
	}
}

int EvolutionaryAlgorithm::compressInternal(VoronoiDiagram * outputDiagram,
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
	cpuFitnessEvaluator->calculateColors(outputDiagram, colors, pixelPointAssignment);

	for (int i = 0; i < population.size(); ++i) {
		delete population[i];
	}

	for (int i = 0; i < diagramPool.size(); ++i) {
		delete diagramPool[i];
	}

	return 0;
}

void EvolutionaryAlgorithm::crossover(
	VoronoiDiagram * firstParent, VoronoiDiagram * secondParent,
	VoronoiDiagram * firstChild, VoronoiDiagram * secondChild) {

	int crossoverStartIndex = Utils::generateRandom(args->diagramPointsCount - 1);
	int crossoverLength = Utils::generateRandom(args->diagramPointsCount - 1) + 1;

	int crossoverLastIndexOverlapping = crossoverStartIndex + crossoverLength;
	int crossoverLastIndex = crossoverLastIndexOverlapping % args->diagramPointsCount;

	for (int i = 0; i < args->diagramPointsCount; ++i) {
		bool isCrossoverIndex = (i >= crossoverStartIndex && i <= crossoverLastIndexOverlapping)
			|| (crossoverLastIndex < crossoverStartIndex && i <= crossoverLastIndex);

		if (isCrossoverIndex) {
			CompressorUtils::copyPoint(firstParent, secondChild, i);
			CompressorUtils::copyPoint(secondParent, firstChild, i);
		}
		else {
			CompressorUtils::copyPoint(firstParent, firstChild, i);
			CompressorUtils::copyPoint(secondParent, secondChild, i);
		}
	}
}