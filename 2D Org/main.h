#include <vector>
#include <string>
using namespace std;


#define HEIGHT 217
#define WIDTH 181
#define BORDER 2

struct Pixel{
	short x, y;
	float f;
	Pixel(short k, short g):x(k), y(g){};
};

bool checkMaskNeighbours(int i, int j, int id, short res);

