#pragma once

#include <string>
#include <cstdint>
#include "compressoralgorithm.h"
#include <string>

namespace lossycompressor {
	
	/// Class compressing image files into voronoi diagram.
	/**
		Currently only images in BMP format with 24 bit color depth are supported.

		Compressed file has following format:
		4 bytes - image width,
		4 bytes - image height,
		2 bytes - image color depth,
		4 bytes - count of points in diagram.
		Rest of the file contains diagram points in following format:
			4 bytes - point x coordinate,
			4 bytes - point y coordinate,
			3 bytes - point color.
	*/
	class Compressor {
	public:
		/// Type of compression computation.
		enum ComputationType {
			LOCAL_SEARCH,	///< Local search.
			EVOLUTIONARY,	///< Evolutionary algorithm.
			MEMETIC			///< Memetic algorithm.
		};

		/// Type of computation limit.
		enum ComputationLimit {
			TIME,			///< Computation is limited by time. Time limit must be passed in arguments.
			FITNESS_COUNT	///< Computation is limited by the count of fitness function evaluation. Limit must be passed in arguments.
		};

		/// Instance of this class is passed as a parameter into Compressor. Contains compression input data and compression settings.
		struct Args {
			const char * sourceImagePath;										///< Path of the source image.
			const char * destinationCompressedPath;								///< Output path of the compressed file.
			const char * destinationImagePath;									///< Output path of the image file reconstructed from the compressed file.
			uint32_t maxCompressedSizeBytes;									///< Maximum size of compressed file in bytes.
			ComputationType computationType = ComputationType::LOCAL_SEARCH;	///< Type of computation.
			ComputationLimit computationLimit = ComputationLimit::TIME;			///< Type of computation limit.
			double maxComputationTimeSecs = 60;									///< Time computation limit.
			int maxFitnessEvaluationCount;										///< Limit on fitness evaluation/
			bool useCuda = false;												///< True if CUDA acceleration should be used, false otherwise.
			char * logFileName = NULL;											///< Path to file into which log of fitness values will be written. Log will be appended to the end of this file. No log will be written if pointer is equal to NULL.
			bool logImprovementToConsole;										///< True if computation should log current fitness into console, false otherwise.
		};
	private:
		const int SUPPORTED_COLOR_DEPTH = 24;
		
		const int COMPRESSED_FILE_HEADER_SIZE = 14;
		const int COMPRESSED_FILE_POINT_POSITION_SIZE = 8;

		const int BITMAP_FILE_HEADER_SIZE = 14;
		const int BITMAP_INFO_HEADER_SIZE = 40;

		Compressor::Args* args;

		// Information from source file's headers
		int32_t sourceWidth;
		int32_t sourceHeight;
		// Raw image data (stored for later writing into the output file)
		int8_t * bitmapFileHeader = NULL;
		int bitmapInfoHeaderAndRestSize;
		// Contains the BIH and rest of data until pixel data start
		int8_t * bitmapInfoHeaderAndRest = NULL;
		// Pixel data stored by rows of pixels from left to right and top to bottom
		int rowWidthInBytes;
		int rowDataWidthInBytes;
		int rowOffsetWidthInBytes;
		uint8_t * sourceImageData = NULL;
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
		static const int ERROR_FILE_COULD_NOT_OPEN_FILE = 2;					///< Compression error code. File could not be open.
		static const int ERROR_FILE_READING_INVALID_BMP_HEADER = 3;				///< Compression error code. File has invalid header.
		static const int ERROR_FILE_READING_UNSUPPORTED_COLOR_DEPTH = 4;		///< Compression error code. Input file has invalid color depth.
		static const int ERROR_FILE_READING_UNSUPPORTED_IMAGE_COMPRESSION = 5;	///< Compression error code. Input file has unsupported image compression.

		/// Construct a new Compressor with given arguments.
		Compressor(Compressor::Args * args) : args(args) {};

		/// Do compression.
		/**
			\return	0 if compression was successfull. Error code if compression was interrupted with error.
		*/
		int compress();
	};
}