#pragma once

#include <cstdint>

namespace lossycompressor {
	/// Represents voronoi diagram.
	/**
		Coordinates of diagram points correspond to pixels of the image.
		Coordinates go from bottom left corner of the image (0, 0) to top
		right corner of the image (imageWidth - 1, imageHeight - 1).
	*/
	struct VoronoiDiagram{
	private:
		bool hasSelfAllocatedPoints;
	public:
		/// Count of points in the diagram.
		const int32_t diagramPointsCount;

		/// X coordinates of points in diagram.
		int32_t * diagramPointsXCoordinates;

		/// Y coordinates of points in diagram.
		int32_t * diagramPointsYCoordinates;

		/// Constructs new diagram.
		/**
			Automatically deallocates arrays of points in the diagram in the destructor.

			param[in] diagramPointsCount	Count of points in the diagram.
		*/
		VoronoiDiagram(int32_t diagramPointsCount)
			: diagramPointsCount(diagramPointsCount),
			diagramPointsXCoordinates(new int32_t[diagramPointsCount]),
			diagramPointsYCoordinates(new int32_t[diagramPointsCount]),
			hasSelfAllocatedPoints(true) {}

		/// Constructs new diagram.
		/**
			Points in the diagram will not be automatically deallocated.

			param[in] diagramPointsCount			Count of points in the diagram.
			param[in] diagramPointsXCoordinates		Pointer to array of X coordinates of points in the diagram.
			param[in] diagramPointsYCoordinates		Pointer to array of Y coordinates of points in the diagram.
		*/
		VoronoiDiagram(int32_t diagramPointsCount, 
			int32_t * diagramPointsXCoordinates, int32_t * diagramPointsYCoordinates)
			: diagramPointsCount(diagramPointsCount),
			diagramPointsXCoordinates(diagramPointsXCoordinates),
			diagramPointsYCoordinates(diagramPointsYCoordinates),
			hasSelfAllocatedPoints(false){}

		~VoronoiDiagram() {
			if (hasSelfAllocatedPoints) {
				delete[] diagramPointsXCoordinates;
				delete[] diagramPointsYCoordinates;
			}
		}

		/// Returns X coordinate of point on given index.
		int32_t x(int index);

		/// Returns Y coordinate of point on given index.
		int32_t y(int index);
	};
}
