#include <vector>

using namespace std;

class pixPoint;
class Edge;

class Vertex{
public:
	vector <pixPoint*> points;
	vector <Edge*> edges;

	Vertex();
	bool contains(pixPoint*);
};