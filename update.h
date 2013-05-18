#include <cmath>

extern float image[HEIGHT][WIDTH][DEPTH]; //image to be segmented
extern short init[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER]; //mask with seed points
//extern short* init[(HEIGHT+BORDER)*(WIDTH+BORDER)*(DEPTH+BORDER)];
extern float phi[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER]; //representation of the zero level set interface
extern short label[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER];//contains only integer values between -3 and 3
extern float F[HEIGHT][WIDTH][DEPTH];

extern list<Pixel> lz; // zero level set
extern list<Pixel> lp1;
extern list<Pixel> ln1;
extern list<Pixel> lp2;
extern list<Pixel> ln2;

extern float treshold, alpha, epsilon;

//temp values
extern list<Pixel> sz; //values in sz are to be moved to lz
extern list<Pixel> sp1;
extern list<Pixel> sn1;
extern list<Pixel> sp2;
extern list<Pixel> sn2;

extern int num_threads;

void prepareUpdates();
void updateLevelSets();
void calculateMu(float);

struct D1{ //first order derivative
	float dx, dy, dz, dxPlus, dyPlus, dzPlus, dxMinus, dyMinus, dzMinus;
	D1(short i, short j, short k){
		dx = (phi[i+1][j][k] - phi[i-1][j][k]) / 2;
		dy = (phi[i][j+1][k] - phi[i][j-1][k]) / 2;
		dz =  (phi[i][j][k+1] - phi[i][j][k-1]) / 2;
		dxPlus  = phi[i+1][j][k] - phi[i][j][k];
		dyPlus  = phi[i][j+1][k] - phi[i][j][k];
		dzPlus  = phi[i][j][k+1] - phi[i][j][k];
		dxMinus = phi[i][j][k] - phi[i-1][j][k];
		dyMinus = phi[i][j][k] - phi[i][j-1][k];
		dzMinus = phi[i][j][k] - phi[i][j][k-1];
	}
};

struct D2 { //second order derivatives
	float dxPlusY, dxPlusZ, dyPlusX, dyPlusZ, dzPlusX, dzPlusY;
	float dxMinusY, dxMinusZ, dyMinusX, dyMinusZ, dzMinusX, dzMinusY;
	D2(short i, short j, short k){
		dxPlusY  = (phi[i+1][j+1][k] - phi[i-1][j+1][k]) / 2;
		dxMinusY = (phi[i+1][j-1][k] - phi[i-1][j-1][k]) / 2;
		dxPlusZ  = (phi[i+1][j][k+1] - phi[i-1][j][k+1]) / 2;
		dxMinusZ = (phi[i+1][j][k-1] - phi[i-1][j][k-1]) / 2;
		dyPlusX  = (phi[i+1][j+1][k] - phi[i+1][j-1][k]) / 2;
		dyMinusX = (phi[i-1][j+1][k] - phi[i-1][j-1][k]) / 2;
		dyPlusZ  = (phi[i][j+1][k+1] - phi[i][j-1][k+1]) / 2;
		dyMinusZ = (phi[i][j+1][k-1] - phi[i][j-1][k-1]) / 2;
		dzPlusX  = (phi[i+1][j][k+1] - phi[i+1][j][k-1]) / 2;
		dzMinusX = (phi[i-1][j][k+1] - phi[i-1][j][k-1]) / 2;
		dzPlusY  = (phi[i][j+1][k+1] - phi[i][j+1][k-1]) / 2;
		dzMinusY = (phi[i][j-1][k+1] - phi[i][j-1][k-1]) / 2;
	}
};

struct Normal{ //normals
	float nPlusX, nPlusY, nPlusZ, nMinusX, nMinusY, nMinusZ;
	Normal(D1 d1, D2 d2){
		nPlusX = d1.dxPlus / sqrt(d1.dxPlus * d1.dxPlus + pow((d2.dyPlusX + d1.dy) / 2, 2) + pow((d2.dzPlusX + d1.dz) / 2, 2));
		nPlusY = d1.dyPlus / sqrt(d1.dyPlus * d1.dyPlus + pow((d2.dxPlusY + d1.dx) / 2, 2) + pow((d2.dzPlusY + d1.dz) / 2, 2));
		nPlusZ = d1.dzPlus / sqrt(d1.dzPlus * d1.dzPlus + pow((d2.dyPlusZ + d1.dx) / 2, 2) + pow((d2.dyPlusZ + d1.dy) / 2, 2));
		nMinusX = d1.dxMinus / sqrt(d1.dxMinus * d1.dxMinus + pow((d2.dyMinusX + d1.dy) / 2, 2) + pow((d2.dzMinusX + d1.dz) / 2, 2));
		nMinusY = d1.dyMinus / sqrt(d1.dyMinus * d1.dyMinus + pow((d2.dxMinusY + d1.dx) / 2, 2) + pow((d2.dzMinusY + d1.dz) / 2, 2));
		nMinusZ = d1.dzMinus / sqrt(d1.dzMinus * d1.dzMinus + pow((d2.dyMinusZ + d1.dx) / 2, 2) + pow((d2.dyMinusZ + d1.dy) / 2, 2));
	}	
};