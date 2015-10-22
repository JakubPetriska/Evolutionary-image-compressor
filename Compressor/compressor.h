#pragma once

#include <string>
#include <cstdint>

struct compressor_args {
	const char * source_image_path;
	const char * destination_compressed_path;
	const char * destination_image_path;
	uint32_t max_compressed_size_bytes;
};

class Compressor {
	const int BITMAP_FILE_HEADER_SIZE = 14;
	const int BITMAP_INFO_HEADER_SIZE = 40;

	compressor_args* compressorArgs;
	
	// Information from source file's headers
	int32_t sourceWidth;
	int32_t sourceHeight;
	uint16_t sourceColorDepth;
	// Raw image data
	int8_t * bitmapFileHeader = NULL;
	int bitmapInfoHeaderAndRestSize = NULL;
	int8_t * bitmapInfoHeaderAndRest;
	// Pixel data stored by rows of pixels from bottom to top
	int rowWidthInBytes;
	uint8_t ** sourceImageData = NULL;

	int readSourceImageFile();
	int writeDestinationImageFile(uint8_t ** imageData);
	void releaseMemory();
public:
	const int ERROR_FILE_COULD_NOT_OPEN_FILE = 2;
	const int ERROR_FILE_READING_INVALID_BMP_HEADER = 3;
	const int ERROR_FILE_READING_UNSUPPORTED_COLOR_DEPTH = 4;

	Compressor(compressor_args* args) : compressorArgs(args) {};
	int compress();
};