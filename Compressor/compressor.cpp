#include "compressor.h"
#include "memeticalgorithm.h"
#include <cstdio>

using namespace lossycompressor;

int Compressor::compress() {
	int err;
	err = readSourceImageFile();
	if (err != 0) {
		printf("Encountered error during source image file reading with code %d\n", err);
		releaseMemory();
		return err;
	}

	CompressorAlgorithm * compressAlgorithm;

	// Prepare the arguments for compression algorithm
	CompressorAlgorithmArgs compressorAlgorithmArgs;
	compressorAlgorithmArgs.sourceWidth = sourceWidth;
	compressorAlgorithmArgs.sourceHeight = sourceHeight;
	compressorAlgorithmArgs.sourceImageData = sourceImageData;
	compressorAlgorithmArgs.sourceDataRowWidthInBytes = rowWidthInBytes;
	compressorAlgorithmArgs.limitByTime = args->computationLimit == ComputationLimit::TIME;
	compressorAlgorithmArgs.maxComputationTimeSecs = args->maxComputationTimeSecs;
	compressorAlgorithmArgs.maxFitnessEvaluationCount = args->maxFitnessEvaluationCount;
	compressorAlgorithmArgs.useCuda = args->useCuda;
	
	// Calculate how many points compressed file can contain
	int compressedFileDataStorageSize = args->maxCompressedSizeBytes - COMPRESSED_FILE_HEADER_SIZE;
	int dataPointSize = COMPRESSED_FILE_POINT_POSITION_SIZE + (SUPPORTED_COLOR_DEPTH / 8);
	compressorAlgorithmArgs.diagramPointsCount = compressedFileDataStorageSize / dataPointSize;

	if (args->computationType == ComputationType::EVOLUTIONARY) {
		compressAlgorithm = new EvolutionaryAlgorithm(&compressorAlgorithmArgs);
	}
	else if (args->computationType == ComputationType::MEMETIC) {
		compressAlgorithm = new MemeticAlgorithm(&compressorAlgorithmArgs);
	}
	else {
		compressAlgorithm = new LocalSearch(&compressorAlgorithmArgs);
	}

	VoronoiDiagram compressedDiagram(compressorAlgorithmArgs.diagramPointsCount);
	Color24bit * diagramColors = new Color24bit[compressorAlgorithmArgs.diagramPointsCount];
	int * pixelPointAssignment = new int[sourceHeight * sourceWidth];

	compressAlgorithm->compress(&compressedDiagram, diagramColors, pixelPointAssignment);
	delete compressAlgorithm;
	
	err = writeCompressedFile(&compressedDiagram, diagramColors);
	if (err != 0) {
		releaseMemory();
		printf("Encountered error during compressed output file writing with code %d\n", err);
		return err;
	}

	// Fill the colors of compressed image into the output data (data that we read as input)
	for (int i = 0; i < sourceHeight; ++i) {
		for (int j = 0; j < sourceWidth; ++j) {
			int pointIndex = pixelPointAssignment[i * sourceWidth + j];
			Color24bit color = diagramColors[pointIndex];
			int colorStartIndexInSourceData = i * rowWidthInBytes + j * 3;

			sourceImageData[colorStartIndexInSourceData] = color.b;
			sourceImageData[colorStartIndexInSourceData + 1] = color.g;
			sourceImageData[colorStartIndexInSourceData + 2] = color.r;
		}
	}

	delete[] diagramColors;
	delete[] pixelPointAssignment;

	err = writeDestinationImageFile();
	if (err != 0) {
		releaseMemory();
		printf("Encountered error during output image file writing with code %d\n", err);
		return err;
	}

	releaseMemory();
	return 0;
}

int Compressor::readSourceImageFile() {
	FILE* file;
	errno_t err = fopen_s(
		&file,
		args->sourceImagePath,
		"rb");
	if (err != 0 || file == NULL) {
		return ERROR_FILE_COULD_NOT_OPEN_FILE;
	}

	// Read 14 bytes of Bitmap File Header
	bitmapFileHeader = new int8_t[BITMAP_FILE_HEADER_SIZE];
	fread(bitmapFileHeader, 1, BITMAP_FILE_HEADER_SIZE, file);

	uint32_t totalFileSize = *(int32_t*)&bitmapFileHeader[2];

	// Figure where pixel data start
	int32_t rawDataStartOffset = *(int32_t*)&bitmapFileHeader[10];

	// Read the rest until pixel data start
	bitmapInfoHeaderAndRestSize = rawDataStartOffset - BITMAP_FILE_HEADER_SIZE;
	bitmapInfoHeaderAndRest = new int8_t[bitmapInfoHeaderAndRestSize];
	fread(bitmapInfoHeaderAndRest, 1, bitmapInfoHeaderAndRestSize, file);
	
	int32_t bitmapInfoHeaderSize = *(int32_t*)&bitmapInfoHeaderAndRest[0];
	if (bitmapInfoHeaderSize != BITMAP_INFO_HEADER_SIZE) {
		fclose(file);
		return ERROR_FILE_READING_INVALID_BMP_HEADER;
	}

	sourceWidth = *(int32_t*)&bitmapInfoHeaderAndRest[4];
	sourceHeight = *(int32_t*)&bitmapInfoHeaderAndRest[8];

	uint16_t sourceColorDepth = *(uint16_t*)&bitmapInfoHeaderAndRest[14];
	if (sourceColorDepth != SUPPORTED_COLOR_DEPTH) {
		fclose(file);
		return ERROR_FILE_READING_UNSUPPORTED_COLOR_DEPTH;
	}

	uint32_t compressionMethod = *(uint16_t*)&bitmapInfoHeaderAndRest[16];
	if (compressionMethod != 0) {
		fclose(file);
		return ERROR_FILE_READING_UNSUPPORTED_IMAGE_COMPRESSION;
	}

	// Read the raw pixel data
	rowWidthInBytes = ((sourceColorDepth * sourceWidth + 31) / 32) * 4;
	rowOffsetWidthInBytes = rowWidthInBytes % 4;
	rowDataWidthInBytes = rowWidthInBytes - rowOffsetWidthInBytes;

	sourceImageData = new uint8_t[sourceHeight * rowDataWidthInBytes];

	// Used to read row offset
	uint8_t * rowOffset = new uint8_t[rowOffsetWidthInBytes];
	// Pixel values are stored by rows from bottom to top, we store them from top to bottom
	for (int i = sourceHeight - 1; i >= 0; --i) {
		fread(sourceImageData + (i * rowWidthInBytes), 1, rowDataWidthInBytes, file);
		if (rowOffsetWidthInBytes > 0) {
			fread(rowOffset, 1, rowOffsetWidthInBytes, file);
		}
	}
	delete[] rowOffset;

	sourceImageFileRestSize = totalFileSize
		- BITMAP_FILE_HEADER_SIZE 
		- bitmapInfoHeaderAndRestSize 
		- (sourceHeight * rowWidthInBytes);
	if (sourceImageFileRestSize > 0) {
		sourceImageFileRest = new int8_t[sourceImageFileRestSize];
		fread(sourceImageFileRest, 1, sourceImageFileRestSize, file);
	}

	fclose(file);
	return 0;
}

int Compressor::writeDestinationImageFile() {
	FILE* file;
	errno_t err = fopen_s(
		&file,
		args->destinationImagePath,
		"wb");
	if (err != 0 || file == NULL) {
		return ERROR_FILE_COULD_NOT_OPEN_FILE;
	}

	fwrite(bitmapFileHeader, 1, BITMAP_FILE_HEADER_SIZE, file);
	fwrite(bitmapInfoHeaderAndRest, 1, bitmapInfoHeaderAndRestSize, file);

	for (int i = sourceHeight - 1; i >= 0; --i) {
		fwrite(sourceImageData + i * rowWidthInBytes, 1, rowDataWidthInBytes, file);
		if (rowOffsetWidthInBytes > 0) {
			fwrite(sourceImageData, 1, rowOffsetWidthInBytes, file);
		}
	}
	
	if (sourceImageFileRestSize > 0) {
		fwrite(sourceImageFileRest, 1, sourceImageFileRestSize, file);
	}

	fflush(file);
	fclose(file);
	return 0;
}

int Compressor::writeCompressedFile(VoronoiDiagram * diagram, Color24bit * colors) {
	FILE* file;
	errno_t err = fopen_s(
		&file,
		args->destinationCompressedPath,
		"wb");
	if (err != 0 || file == NULL) {
		return ERROR_FILE_COULD_NOT_OPEN_FILE;
	}

	// TODO file is smaller than should be
	int8_t * outputData = new int8_t[14];
	((int32_t *)outputData)[0] = sourceWidth;
	((int32_t *)outputData)[1] = sourceHeight;
	((int16_t *)outputData)[4] = 24;
	((int32_t *)&(outputData[10]))[0] = diagram->diagramPointsCount;

	fwrite(outputData, 1, 14, file);
	
	for (int i = 0; i < diagram->diagramPointsCount; ++i) {
		((int32_t *)outputData)[0] = diagram->x(i);
		((int32_t *)outputData)[1] = diagram->y(i);
		((uint8_t *)outputData)[8] = colors[i].b;
		((uint8_t *)outputData)[9] = colors[i].g;
		((uint8_t *)outputData)[10] = colors[i].r;

		fwrite(outputData, 1, 11, file);
	}

	fflush(file);
	fclose(file);
	return 0;
}

void Compressor::releaseMemory() {
	if (bitmapFileHeader != NULL) {
		delete[] bitmapFileHeader;
	}
	if (bitmapInfoHeaderAndRest != NULL) {
		delete[] bitmapInfoHeaderAndRest;
	}
	if (sourceImageData != NULL) {
		delete[] sourceImageData;
	}
}