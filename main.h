#include <list>
using namespace std;

#define HEIGHT 256
#define WIDTH 256
#define DEPTH 192
#define BORDER 2

struct Pixel{
	short x, y, z;
	float f;
	Pixel(short k, short g, short u):x(k), y(g), z(u){};

};

bool checkMaskNeighbours(int i, int j, int k, int id, short res);

