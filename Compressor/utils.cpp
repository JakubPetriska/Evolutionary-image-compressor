#include "utils.h"
#include <chrono>

using namespace lossycompressor;

void Utils::getCurrentMillis(__int64 * out) {
	*out = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}

void Utils::swap(int32_t * arr, int firstIndex, int secondIndex) {
	int32_t tmp = arr[firstIndex];
	arr[firstIndex] = arr[secondIndex];
	arr[secondIndex] = tmp;
}

int Utils::max(int first, int second) {
	if (first > second) {
		return first;
	}
	else {
		return second;
	}
}