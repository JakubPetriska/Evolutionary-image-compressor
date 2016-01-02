#include <cstdio>
#include <random>
#include "compressor.h"
#include "utils.h"

using namespace std;
using namespace lossycompressor;

int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("Usage: source_image_file_path compressed_file_path compressed_image_file_path max_size_in_bytes\n");
		return 1;
	}

	CompressorArgs compressorArgs;
	compressorArgs.sourceImagePath = argv[1];
	compressorArgs.destinationCompressedPath = argv[2];
	compressorArgs.destinationImagePath = argv[3];
	compressorArgs.maxCompressedSizeBytes = atoi(argv[4]);
	compressorArgs.computationType = ComputationType::LOCAL_SEARCH;
	compressorArgs.computationLimit = ComputationLimit::FITNESS_COUNT;
	compressorArgs.maxComputationTimeSecs = 5 * 60;
	compressorArgs.maxFitnessEvaluationCount = 10;//100000 * 2;
	compressorArgs.useCuda = true;

	Compressor compressor(&compressorArgs);

	LARGE_INTEGER startTime, endTime;
	Utils::recordTime(&startTime);
	int compressionResult = compressor.compress();

	Utils::recordTime(&endTime);
	double calculationTotalTime = Utils::calculateInterval(&startTime, &endTime);
	if (compressionResult == 0) {
		printf("Compressing took %.4f seconds\n", calculationTotalTime);
	}

	return compressionResult;
}