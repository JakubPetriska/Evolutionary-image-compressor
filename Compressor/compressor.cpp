#include "compressor.h"
#include "compressoralgorithms.h"
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
	
	// Calculate how many points compressed file can contain
	int compressedFileDataStorageSize = args->maxCompressedSizeBytes - COMPRESSED_FILE_HEADER_SIZE;
	int dataPointSize = COMPRESSED_FILE_POINT_POSITION_SIZE + SUPPORTED_COLOR_DEPTH;
	compressorAlgorithmArgs.diagramPointsCount = compressedFileDataStorageSize / dataPointSize;

	compressAlgorithm = new LocalSearch(&compressorAlgorithmArgs);

	VoronoiDiagram compressedDiagram(compressorAlgorithmArgs.diagramPointsCount);
	Color24bit * diagramColors = new Color24bit[compressorAlgorithmArgs.diagramPointsCount];
	int ** pixelPointAssignment = new int*[sourceHeight];
	for (int i = 0; i < sourceHeight; ++i) {
		pixelPointAssignment[i] = new int[sourceWidth];
	}

	compressAlgorithm->compress(&compressedDiagram, diagramColors, pixelPointAssignment);
	delete compressAlgorithm;
	
	// TODO write the compressed diagram into output file

	for (int i = 0; i < sourceHeight; ++i) {
		uint8_t * row = sourceImageData[i];
		for (int j = 0; j < sourceWidth; ++j) {
			int pointIndex = pixelPointAssignment[i][j];
			Color24bit color = diagramColors[pointIndex];
			int colorStartIndexInSourceData = j * 3;

			row[colorStartIndexInSourceData] = color.b;
			row[colorStartIndexInSourceData + 1] = color.g;
			row[colorStartIndexInSourceData + 2] = color.r;
		}
	}

	delete[] diagramColors;

	err = writeDestinationImageFile();
	if (err != 0) {
		releaseMemory();
		printf("Encountered error during output image file writing with code %d\n", err);
		return err;
	}

	releaseMemory();
	return err;
}

void Compressor::releaseMemory() {
	if (bitmapFileHeader != NULL) {
		delete[] bitmapFileHeader;
	}
	if (bitmapInfoHeaderAndRest != NULL) {
		delete[] bitmapInfoHeaderAndRest;
	}
	if (sourceImageData != NULL) {
		for (int i = 0; i < sourceHeight; ++i) {
			delete[] sourceImageData[i];
		}
		delete[] sourceImageData;
	}
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
	sourceImageData = new uint8_t*[sourceHeight];
	for (int i = 0; i < sourceHeight; ++i) {
		uint8_t * newRow = new uint8_t[rowWidthInBytes];
		fread(newRow, 1, rowWidthInBytes, file);
		sourceImageData[i] = newRow;
	}

	sourceImageFileRestSize = totalFileSize
		- BITMAP_FILE_HEADER_SIZE 
		- bitmapInfoHeaderAndRestSize 
		- (sourceHeight * rowWidthInBytes);
	if (sourceImageFileRestSize > 0) {
		sourceImageFileRest = new int8_t[sourceHeight];
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

	for (int i = 0; i < sourceHeight; ++i) {
		fwrite(sourceImageData[i], 1, rowWidthInBytes, file);
	}
	
	if (sourceImageFileRestSize > 0) {
		fwrite(sourceImageFileRest, 1, sourceImageFileRestSize, file);
	}

	fflush(file);
	fclose(file);
	return 0;
}