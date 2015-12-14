
namespace lossycompressor {

	class FitnessCalculator {
	public:
		virtual ~FitnessCalculator() {}
		virtual float calculateFitness(VoronoiDiagram * diagram) = 0;
	};
}