#include <cstdio>
#include <random>
#include "compressor.h"
#include "utils.h"

using namespace std;
using namespace lossycompressor;

#define ITERATION_COUNT 15

int doTestIteration(ComputationType computationType, char * logFileName) {
	string inputFileName = "Mona_Lisa";
	//string inputFileName = "abstraktni_krivky";
	//string inputFileName = "kubismus_krajina";

	string fileFormatImage = ".bmp";
	string fileFormatVor = ".vor";
	string folderInput = "test_images/";
	string folderOutput = "test_output/";

	string sourceImagePath = folderInput + inputFileName + fileFormatImage;
	string destinationCompressedPath = folderOutput + inputFileName + fileFormatVor;
	string destinationImagePath = folderOutput + inputFileName + fileFormatImage;

	CompressorArgs compressorArgs;
	compressorArgs.sourceImagePath = sourceImagePath.c_str();
	compressorArgs.destinationCompressedPath = destinationCompressedPath.c_str();
	compressorArgs.destinationImagePath = destinationImagePath.c_str();
	compressorArgs.maxCompressedSizeBytes = 4000;
	compressorArgs.computationType = computationType;
	compressorArgs.computationLimit = ComputationLimit::FITNESS_COUNT;
	compressorArgs.maxComputationTimeSecs = 60 * 10;
	compressorArgs.maxFitnessEvaluationCount = 30;
	compressorArgs.useCuda = true;
	compressorArgs.logFileName = logFileName;
	compressorArgs.logImprovementToConsole = false;

	LARGE_INTEGER startTime, endTime;
	Utils::recordTime(&startTime);

	Compressor compressor(&compressorArgs);
	int compressionResult = compressor.compress();

	Utils::recordTime(&endTime);
	double calculationTotalTime = Utils::calculateInterval(&startTime, &endTime);
	if (compressionResult == 0) {
		printf("Compressing took %.4f seconds\n", calculationTotalTime);
	}

	return compressionResult;
}

int main(int argc, char* argv[]) {
	//for (int i = 0; i < ITERATION_COUNT; ++i) {
	//	doTestIteration(argv, "log_test.csv", ComputationType::MEMETIC);
	//}
	doTestIteration(ComputationType::LOCAL_SEARCH, NULL);
}

//int main(int argc, char* argv[]) {
//	if (argc < 4) {
//		printf("Usage: source_image_file_path compressed_file_path compressed_image_file_path max_size_in_bytes\n");
//		return 1;
//	}
//
//	char * logFileName = "log_Local_search.csv";
//
//	CompressorArgs compressorArgs;
//	compressorArgs.sourceImagePath = argv[1];
//	compressorArgs.destinationCompressedPath = argv[2];
//	compressorArgs.destinationImagePath = argv[3];
//	compressorArgs.maxCompressedSizeBytes = atoi(argv[4]);
//	compressorArgs.computationType = ComputationType::LOCAL_SEARCH;
//	compressorArgs.computationLimit = ComputationLimit::FITNESS_COUNT;
//	compressorArgs.maxComputationTimeSecs = 60 * 10;
//	compressorArgs.maxFitnessEvaluationCount = 10;
//	compressorArgs.useCuda = true;
//	compressorArgs.logFileName = logFileName;
//	compressorArgs.logImprovementToConsole = false;
//
//	Compressor compressor(&compressorArgs);
//
//	LARGE_INTEGER startTime, endTime;
//	Utils::recordTime(&startTime);
//	int compressionResult = compressor.compress();
//
//	Utils::recordTime(&endTime);
//	double calculationTotalTime = Utils::calculateInterval(&startTime, &endTime);
//	if (compressionResult == 0) {
//		printf("Compressing took %.4f seconds\n", calculationTotalTime);
//	}
//
//	return compressionResult;
//}