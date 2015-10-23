#include "compressor.h"
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
	
	err = writeDestinationImageFile(sourceImageData);
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
		compressorArgs->source_image_path,
		"rb");
	if (err != 0 || file == NULL) {
		return ERROR_FILE_COULD_NOT_OPEN_FILE;
	}

	// Read 14 bytes of Bitmap File Header
	bitmapFileHeader = new int8_t[BITMAP_FILE_HEADER_SIZE];
	fread(bitmapFileHeader, 1, BITMAP_FILE_HEADER_SIZE, file);

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
	sourceColorDepth = *(uint16_t*)&bitmapInfoHeaderAndRest[14];

	if (sourceColorDepth != 24) {
		fclose(file);
		return ERROR_FILE_READING_UNSUPPORTED_COLOR_DEPTH;
	}
	// TODO only supports 24 bits color depth (8 bits each R, G and B)

	// Read the raw pixel data
	rowWidthInBytes = sourceWidth * (sourceColorDepth / 8);
	sourceImageData = new uint8_t*[sourceHeight];
	for (int i = 0; i < sourceHeight; ++i) {
		uint8_t * newRow = new uint8_t[rowWidthInBytes];
		fread(newRow, 1, rowWidthInBytes, file);
		sourceImageData[i] = newRow;
	}

	fclose(file);
	return 0;
}

int Compressor::writeDestinationImageFile(uint8_t ** imageData) {
	FILE* file;
	errno_t err = fopen_s(
		&file,
		compressorArgs->destination_image_path,
		"wb");
	if (err != 0 || file == NULL) {
		return ERROR_FILE_COULD_NOT_OPEN_FILE;
	}

	fwrite(bitmapFileHeader, 1, BITMAP_FILE_HEADER_SIZE, file);
	fwrite(bitmapInfoHeaderAndRest, 1, bitmapInfoHeaderAndRestSize, file);

	for (int i = 0; i < sourceHeight; ++i) {
		fwrite(sourceImageData[i], 1, rowWidthInBytes, file);
	}
	fflush(file);
	fclose(file);
	return 0;
}