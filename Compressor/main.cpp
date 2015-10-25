#include <cstdio>
#include "compressor.h"

using namespace std;
using namespace lossycompressor;

int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("Usage: source_image_file_path compressed_file_path compressed_image_file_path max_size_in_bytes");
		return 1;
	}

	CompressorArgs compressorArgs;
	compressorArgs.sourceImagePath = argv[1];
	compressorArgs.destinationCompressedPath = argv[2];
	compressorArgs.destinationImagePath = argv[3];
	compressorArgs.maxCompressedSizeBytes = atoi(argv[4]);

	Compressor compressor(&compressorArgs);
	compressor.compress();
}