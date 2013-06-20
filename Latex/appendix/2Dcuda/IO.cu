#include "IO.h"
#include "main.h"
#include <sstream> 
using namespace std;

extern int iterations;
extern float threshold, alpha, epsilon;
extern float image[HEIGHT][WIDTH];
extern int label[HEIGHT][WIDTH];
extern int zeroLevelSet[HEIGHT][WIDTH];

//read input file
void readFile(BMP img){
	//copy input data (img) to image[][] and normalize to [0, 1]
	for (int i =0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			image[i][j] = (img(i,j)->Red + img(i,j)->Green + img(i,j)->Blue) / 3;
			image[i][j] /= 255;
		}
	}
}

void writeFile(BMP img, int id, int iterations){
	string name;
	stringstream sstm;
	if(id==1){
		for (short i =0; i<HEIGHT; i++){
			for (short j = 0; j<WIDTH; j++){
				img(i,j)->Red = (label[i][j] +3)*42; //normalize to [0, 255]
				img(i,j)->Green = (label[i][j] +3)*42;
				img(i,j)->Blue = (label[i][j] +3)*42;
			}
		}
		
		sstm << iterations << "label" <<".bmp";
		name = sstm.str();
		img.WriteToFile(name.c_str());
		printf("\nlabel image stored");
	}
	else{ //zeroLevelSet
		for (short i =0; i<HEIGHT; i++){
			for (short j = 0; j<WIDTH; j++){
				img(i,j)->Red = zeroLevelSet[i][j]; 
				img(i,j)->Green = zeroLevelSet[i][j]; 
				img(i,j)->Blue = zeroLevelSet[i][j]; 
			}
		}
		sstm << iterations << "zero" <<".bmp";
		name = sstm.str();
		img.WriteToFile(name.c_str());
		printf("\nzero image stored\n");
	}
}

bool getAndVerifyInput(int argc, char *argv[]){
	if(argc != 5){
		printf("Need four inputs: iterations, threshold, epsilon, alpha \n");
		return false;
	}
	if(sscanf (argv[1], "%i", & iterations)!=1 || iterations<0) {
		printf("Need four inputs: iterations, threshold, epsilon, alpha \n");
		return false;
	}
	if(sscanf(argv[2], "%f", &threshold)!=1 || threshold >1){
		printf("Need four inputs: iterations, threshold, epsilon, alpha \n");
		return false;
	}
	if(sscanf(argv[3], "%f", &epsilon)!=1 || epsilon >1){
		printf("Need four inputs: iterations, threshold, epsilon, alpha \n");
		return false;
	}
	if(sscanf(argv[4], "%f", &alpha)!=1 || alpha>1){
		printf("Need four inputs: iterations, threshold, epsilon, alpha \n");
		return false;
	}
	return true;
}