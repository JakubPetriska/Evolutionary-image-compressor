#include "utils.h"
#include <cmath>
#include <random>

using namespace std;
using namespace lossycompressor;

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

double Utils::calculateSquareDistance(int32_t firstX, int32_t firstY, int32_t secondX, int32_t secondY) {
	return pow(firstX - secondX, 2) + pow(firstY - secondY, 2);
}

void Utils::recordTime(LARGE_INTEGER * event) {
	QueryPerformanceCounter(event);
}

double Utils::calculateInterval(LARGE_INTEGER * start, LARGE_INTEGER * end) {
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return static_cast<double>(end->QuadPart - start->QuadPart) / frequency.QuadPart;
}

int Utils::generateRandom(int max) {
	random_device rd;
	double m = max / (double)(rd.max() - rd.min());
	unsigned int randVal = rd();
	return (int) ((randVal - rd.min()) * m);
}
