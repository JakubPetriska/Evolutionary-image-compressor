#include "compressor.h"
#include "compressoralgorithms.h"

using namespace lossycompressor;

void CompressorAlgorithm::calculateColors(VoronoiDiagram * diagram, Color24bit * colors) {
	for (int i = 0; i < args->diagramPointsCount; ++i) {
		rSums[i] = 0;
		rCounts[i] = 0;
		gSums[i] = 0;
		gCounts[i] = 0;
		bSums[i] = 0;
		bCounts[i] = 0;
	}
	for (int i = 0; i < args->sourceHeight; ++i) {
		uint8_t * row = args->sourceImageData[i];
		for (int j = 0; j < args->sourceWidth; ++j) {
			// TODO determine index of point to which the pixel belongs
			int pointIndex = 0;

			int colorStartIndexInSourceData = j * 3;
			rSums[pointIndex] += row[colorStartIndexInSourceData];
			rCounts[pointIndex] += 1;
			gSums[pointIndex] += row[colorStartIndexInSourceData + 1];
			gCounts[pointIndex] += 1;
			bSums[pointIndex] += row[colorStartIndexInSourceData + 2];
			bCounts[pointIndex] += 1;
		}
	}
	// TODO fill the colors array
	for (int i = 0; i < args->diagramPointsCount; ++i) {
		Color24bit * color = color
	}
}

float CompressorAlgorithm::calculateFitness(VoronoiDiagram * diagram) {
	calculateColors(diagram, colorsTmp);
	// TODO calculate fitness
	return 0;
}

int LocalSearch::compress(VoronoiDiagram * outputDiagram, Color24bit * colors) {
	for () {
		
	}
	return 0;
}