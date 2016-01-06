#pragma once

#include <cstdint>

namespace lossycompressor {

	/// Represents a color in RGB format with depth of 24 bits.
	struct Color24bit {
		uint8_t b;
		uint8_t g;
		uint8_t r;
	};
}