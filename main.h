#include <vector>
#include <string>
using namespace std;


#define HEIGHT 512
#define WIDTH 512
#define DEPTH 512
#define BORDER 2

struct Pixel{
	int x, y, z;
	Pixel(int k, int g, int f):x(k), y(g), z(f){};

};

bool checkMaskNeighbours(int i, int j, int id, int res);

