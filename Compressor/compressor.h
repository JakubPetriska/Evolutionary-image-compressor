#pragma once

#include <string>
#include <cstdint>
#include "compressoralgorithms.h"

/*
	Class compressing image files on given paths into voronoi diagram.

	Currently only images in BMP format with 24 bit color depth are supported.

	Compressed file has following format:
		4 bytes - image width
		4 bytes - image height
		2 bytes - image color depth
		4 bytes - count of points in diagram
		Rest of the file contains diagram points in following format:
			4 bytes - point x coordinate
			4 bytes - point y coordinate
			3 bytes - point color
*/

namespace lossycompressor {

	const int COMPRESSED_FILE_HEADER_SIZE = 14;
	const int COMPRESSED_FILE_POINT_POSITION_SIZE = 8;

	enum ComputationType {LOCAL_SEARCH, EVOLUTIONARY};

	struct CompressorArgs {
		const char * sourceImagePath;
		const char * destinationCompressedPath;
		const char * destinationImagePath;
		uint32_t maxCompressedSizeBytes;
		ComputationType computationType = ComputationType::LOCAL_SEARCH;
		double maxComputationTimeSecs = 60;
	};

	class Compressor {
		const int BITMAP_FILE_HEADER_SIZE = 14;
		const int BITMAP_INFO_HEADER_SIZE = 40;

		CompressorArgs* args;

		// Information from source file's headers
		int32_t sourceWidth;
		int32_t sourceHeight;
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
		int writeDestinationImageFile();
		int writeCompressedFile(VoronoiDiagram * diagram, Color24bit * colors);
		void releaseMemory();
	public:
		const int SUPPORTED_COLOR_DEPTH = 24;

		const int ERROR_FILE_COULD_NOT_OPEN_FILE = 2;
		const int ERROR_FILE_READING_INVALID_BMP_HEADER = 3;
		const int ERROR_FILE_READING_UNSUPPORTED_COLOR_DEPTH = 4;
		const int ERROR_FILE_READING_UNSUPPORTED_IMAGE_COMPRESSION = 5;

		Compressor(CompressorArgs * args) : args(args) {};
		int compress();
	};
}