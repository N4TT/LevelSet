#include <vector>
#include <string>
using namespace std;


#define HEIGHT 256
#define WIDTH 256
#define DEPTH 256
#define BORDER 2

struct Pixel{
	unsigned int x, y, z;
	Pixel(unsigned int k, unsigned int g, unsigned int f):x(k), y(g), z(f){};

};

bool checkMaskNeighbours(int i, int j, int k, int id, int res);

