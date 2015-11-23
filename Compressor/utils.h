#pragma once

#include <cstdint>

#define NOMINMAX
#include "Windows.h"

namespace lossycompressor {
	class Utils {
	public:
		static void swap(int32_t * arr, int firstIndex, int secondIndex);
		static int max(int first, int second);
		static double calculateSquareDistance(int32_t firstX, int32_t firstY, 
			int32_t secondX, int32_t secondY);

		static void recordTime(LARGE_INTEGER * event);
		static double calculateInterval(LARGE_INTEGER * start, LARGE_INTEGER * end);
		static int generateRandom(int max);
	};
}