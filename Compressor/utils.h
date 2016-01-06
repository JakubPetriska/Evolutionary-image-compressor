#pragma once

#include <cstdint>

#define NOMINMAX
#include "Windows.h"

namespace lossycompressor {

	/// Contains general utility methods.
	class Utils {
	public:
		/// Swap two data items in an array.
		static void swap(int32_t * arr, int firstIndex, int secondIndex);
		
		/// Return the greater value out of the two argument.
		static int max(int first, int second);
		
		/// Calculate squared distance between the two points.
		/**
			\param[in] firstX	X coordinate of the first point.
			\param[in] firstY	Y coordinate of the first point.
			\param[in] secondX	X coordinate of the second point.
			\param[in] secondY	Y coordinate of the second point.
		*/
		static double calculateSquareDistance(int firstX, int firstY,
			int secondX, int secondY);

		/// Record current time using the Windows API.
		/**
			\param[in] event	Variable pointer into which current time will be recorded.
		*/
		static void recordTime(LARGE_INTEGER * event);
		
		/// Calculate time interval between two events.
		static double calculateInterval(LARGE_INTEGER * start, LARGE_INTEGER * end);
		
		/// Generate random integer between  and max inclusive.
		static int generateRandom(int max);
	};
}