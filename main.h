#include <vector>
#include <string>
using namespace std;


#define HEIGHT 256
#define WIDTH 256
#define DEPTH 256
#define BORDER 2

struct Pixel{
	short x, y, z;
	float f;
	Pixel(short k, short g, short f):x(k), y(g), z(f){};

};

bool checkMaskNeighbours(int i, int j, int k, int id, short res);

