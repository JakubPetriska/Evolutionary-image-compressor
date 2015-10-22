#include <cstdio>
#include "compressor.h"

using namespace std;

int main(int argc, char* argv[]) {
	if (argc < 4) {
		printf("Usage: source_image_file_path compressed_file_path compressed_image_file_path max_size_in_bytes");
		return 1;
	}

	compressor_args compressorArgs;
	compressorArgs.source_image_path = argv[1];
	compressorArgs.destination_compressed_path = argv[2];
	compressorArgs.destination_image_path = argv[3];
	compressorArgs.max_compressed_size_bytes = atoi(argv[4]);

	printf("%s\n", compressorArgs.source_image_path);
	printf("%s\n", compressorArgs.destination_compressed_path);
	printf("%s\n", compressorArgs.destination_image_path);
	printf("%d\n", compressorArgs.max_compressed_size_bytes);

	Compressor compressor(&compressorArgs);
	compressor.compress();
}