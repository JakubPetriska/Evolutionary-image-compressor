#include "utils.h"
#include <chrono>

using namespace lossycompressor;

void Utils::getCurrentMillis(__int64 * out) {
	*out = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
}