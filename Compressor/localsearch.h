#pragma once

#include "compressor.h"

namespace lossycompressor {

	class LocalSearch : public CompressorAlgorithm {
	public:
		// TODO shouldn't it be virtual?
		int compress(CompressorAlgorithmArgs* args);
	};
}