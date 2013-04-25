extern float image[HEIGHT][WIDTH]; //image to be segmented
extern short init[HEIGHT+BORDER][WIDTH+BORDER]; //mask with seed points
extern float phi[HEIGHT+BORDER][WIDTH+BORDER]; //representation of the zero level set interface
extern short label[HEIGHT+BORDER][WIDTH+BORDER];//contains only integer values between -3 and 3
extern float F[HEIGHT][WIDTH];

extern vector<Pixel> lz; // zero level set
extern vector<Pixel> lp1;
extern vector<Pixel> ln1;
extern vector<Pixel> lp2;
extern vector<Pixel> ln2;

extern float treshold, alpha, epsilon;

//temp values
extern vector<Pixel> sz; //values in sz are to be moved to lz
extern vector<Pixel> sp1;
extern vector<Pixel> sn1;
extern vector<Pixel> sp2;
extern vector<Pixel> sn2;

void prepareUpdates();
void updateLevelSets();
void calculateMu();

struct D1{ //first order derivative
	float dx, dy, dxPlus, dyPlus, dxMinus, dyMinus;
	D1(short i, short j){
		dx = (phi[i+1][j] - phi[i-1][j]) / 2;
		dy = (phi[i][j+1] - phi[i][j-1]) / 2;
		dxPlus = phi[i+1][j] - phi[i][j];
		dyPlus = phi[i][j+1] - phi[i][j];
		dxMinus = phi[i][j] - phi[i-1][j];
		dyMinus = phi[i][j] - phi[i][j-1];
	}
};

struct D2{ //second order derivatives
	float dxPlusY, dxMinusY, dyPlusX, dyMinusX;
	D2(short i, short j){
		dxPlusY = (phi[i+1][j+1] - phi[i-1][j+1])/2;
		dxMinusY = (phi[i+1][j-1] - phi[i-1][j-1])/2;
		dyPlusX = (phi[i+1][j+1] - phi[i+1][j-1])/2;
		dyMinusX = (phi[i-1][j+1] - phi[i-1][j-1])/2;
	}
};

struct Normal{ //normals
	float nPlusX, nPlusY, nMinusX, nMinusY;
	Normal(D1 d1, D2 d2){
		nPlusX = d1.dxPlus / sqrt(d1.dxPlus*d1.dxPlus + pow((d2.dyPlusX + d1.dy) / 2, 2));
		nPlusY = d1.dyPlus / sqrt(d1.dyPlus*d1.dyPlus + pow((d2.dxPlusY + d1.dx) / 2, 2));
		nMinusX = d1.dxMinus / sqrt(d1.dxMinus * d1.dxMinus + pow((d2.dyMinusX + d1.dy) / 2, 2));
		nMinusY = d1.dyMinus / sqrt(d1.dyMinus * d1.dyMinus + pow((d2.dxMinusY + d1.dx) / 2, 2));
	}	
};