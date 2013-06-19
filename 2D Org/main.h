//#include <vector>
#include <list>
#include <string>
using namespace std;


#define HEIGHT 512
#define WIDTH 512
#define BORDER 2

struct Pixel{
	short x, y;
	Pixel(short k, short g):x(k), y(g){};
};

bool checkMaskNeighbours(short i, short j, int id, short res);

