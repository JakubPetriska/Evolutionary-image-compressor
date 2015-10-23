#pragma once

#include <string>
#include <cstdint>

namespace lossycompressor {
	struct CompressorArgs {
		const char * sourceImagePath;
		const char * destinationCompressedPath;
		const char * destinationImagePath;
		uint32_t maxCompressedSizeBytes;
	};

	class Compressor {
		const int BITMAP_FILE_HEADER_SIZE = 14;
		const int BITMAP_INFO_HEADER_SIZE = 40;

		CompressorArgs* compressorArgs;

		// Information from source file's headers
		int32_t sourceWidth;
		int32_t sourceHeight;
		uint16_t sourceColorDepth;
		// Raw image data (stored for later writing into the output file)
		int8_t * bitmapFileHeader = NULL;
		int bitmapInfoHeaderAndRestSize;
		// Contains the BIH and rest of data until pixel data start
		int8_t * bitmapInfoHeaderAndRest = NULL;
		// Pixel data stored by rows of pixels from bottom to top
		int rowWidthInBytes;
		uint8_t ** sourceImageData = NULL;
		// Rest of the file after pixel data
		int sourceImageFileRestSize;
		int8_t * sourceImageFileRest = NULL;

		// Compressed image representation
		void * compressedImage;

		int readSourceImageFile();
		int writeDestinationImageFile(uint8_t ** imageData);
		void releaseMemory();
	public:
		const int ERROR_FILE_COULD_NOT_OPEN_FILE = 2;
		const int ERROR_FILE_READING_INVALID_BMP_HEADER = 3;
		const int ERROR_FILE_READING_UNSUPPORTED_COLOR_DEPTH = 4;
		const int ERROR_FILE_READING_UNSUPPORTED_IMAGE_COMPRESSION = 5;

		Compressor(CompressorArgs* args) : compressorArgs(args) {};
		int compress();
	};

	struct CompressorAlgorithmArgs {
		int32_t sourceWidth;
		int32_t sourceHeight;
		uint8_t ** sourceImageData;
		int32_t compressedDataSize;
		void * compressedData;
	};

	// TODO only uses fixed color depth of 24 bits
	class CompressorAlgorithm {
	public:
		virtual int compress(CompressorAlgorithmArgs* args) = 0;
	};
}