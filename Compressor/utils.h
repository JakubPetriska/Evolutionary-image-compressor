#pragma once

#include <cstdint>

namespace lossycompressor {
	class Utils {
	public:
		static void getCurrentMillis(__int64 * out);
		static void swap(int32_t * arr, int firstIndex, int secondIndex);
		static int max(int first, int second);
	};
}