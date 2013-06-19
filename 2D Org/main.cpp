#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"
#include <list>
#include "EasyBMP.h" //library for reading bmp files
#include "update.h" //levelset process happens here
//#include "IO.h" //handles input and stores output
#include <cstdio> //to calculate runtime
#include <ctime>  //to calculate runtime

#include <sstream> 
using namespace std;

//to calculate runtime
clock_t start;
double duration;

float image[HEIGHT][WIDTH] = { 0 }; //input -> image to be segmented
float phi[HEIGHT+BORDER][WIDTH+BORDER] = { 0 }; //level set
short init[HEIGHT+BORDER][WIDTH+BORDER] = { 0 }; //binary mask with seed points
int label[HEIGHT+BORDER][WIDTH+BORDER] = { 0 }; //contains info about the layers
int zeroLevelSet[HEIGHT][WIDTH] = { 0 }; //output

int iterations;
float threshold, alpha, epsilon;

list<Pixel> lz;
list<Pixel> lp1;
list<Pixel> ln1;
list<Pixel> lp2;
list<Pixel> ln2;

list<Pixel> sz;
list<Pixel> sp1;
list<Pixel> sn1;
list<Pixel> sp2;
list<Pixel> sn2;

//fills init with circular seed point, returns 1 if success
int fillSphere(int seedX, int seedY, int radius){
	//(seedX, seedY) -> coordinates - center of seed point
	if(seedX < 0 || seedX > HEIGHT || seedY < 0 || seedY > WIDTH){
		printf("Wrong input to create a circular seed\n");
		printf("Coordinates out of range\n");
		return 0;
	}
	else if(radius < 1 || radius > HEIGHT/2 || radius > WIDTH/2){
		printf("Wrong input to create a circular seed\n");
		printf("Radius must be a positive integer less than min(width, height)/2\n");
		return 0;
	}
	for(int i = seedX - radius; i < seedX + radius; i++){
		for(int j = seedY - radius; j < seedY + radius; j++){
			if(sqrt((float)((seedX-i)*(seedX-i)+(seedY-j)*(seedY-j))) < radius){
				init[i][j] = 1;
			}
		}
	}
	return 1;
}

//can replace fillSphere() if a rectangular seed point is wanted
int fillInit(short minX, short minY, short maxX, short maxY){
	//(minX, minY) -> upper left corner
	//(maxX, maxY) -> lower right corner
	if(maxX - minX <= 0 || maxY - minY <= 0){
		printf("Wrong input to create a rectangular seed\n");
		printf("Input must be in this order minX minY maxX maxY\n");
		return 0;
	}
	else if(minX < 0 || maxX >= HEIGHT || minY < 0 || maxY >= WIDTH){
		printf("Wrong input to create a rectangular seed\n");
		printf("Input out of range\n");
		return 0;
	}
	for (int i = minY+1; i<maxY+1; i++){
		for (int j = minX+1; j<maxX+1; j++){
			init[i][j] = 1;
		}
	}
	return 1;
}

/* returns true if any neighbour of coordinates (i,j) in either
   init[][] (id = 1) or label[][] (id = 2) equals res */
bool checkMaskNeighbours(short i, short j, int id, short res){
	if(id == 1){ //id == 1 -> init
		if(init[i+1][j] == res)
			return true;
		else if(init[i-1][j] == res)
			return true;
		else if(init[i][j+1] == res)
			return true;
		else if(init[i][j-1] == res)
			return true;
	}
	else if(id == 2){ //id == 2 -> label
		if(label[i+1][j] == res)
			return true;
		else if(label[i-1][j] == res)
			return true;
		else if(label[i][j+1] == res)
			return true;
		else if(label[i][j-1] == res)
			return true;
	}
	return false;
}

//add pixels to lists according to their label
void assignLabel(Pixel p, short level){
	switch(level){
	case 1:
		lp1.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;
		break;
	case 2:
		lp2.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;
		break;
	case -1:
		ln1.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;
		break;
	case -2:
		ln2.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;	
		break;
	}
}

void setLevels(Pixel p, short level){
	if(label[p.x+1][p.y] == 3){
		assignLabel(Pixel(p.x+1, p.y), level);
	}
	if(label[p.x][p.y+1] == 3){
		assignLabel(Pixel(p.x, p.y+1), level);
	}
	if(label[p.x-1][p.y] == 3){
		assignLabel(Pixel(p.x-1, p.y), level);
	}
	if(label[p.x][p.y-1] == 3){
		assignLabel(Pixel(p.x, p.y-1), level);
	}
	if(label[p.x+1][p.y] == -3){
		assignLabel(Pixel(p.x+1, p.y), -level);
	}
	if(label[p.x][p.y+1] == -3){
		assignLabel(Pixel(p.x, p.y+1), -level);
	}
	if(label[p.x-1][p.y] == -3){
		assignLabel(Pixel(p.x-1, p.y), -level);
	}
	if(label[p.x][p.y-1] == -3){
		assignLabel(Pixel(p.x, p.y-1), -level);
	}
}	

//initializes Ln2, Ln1, Lz, Lp1, Lp2 based on seed point(s)
void initialization(){
	list<Pixel>::iterator it;

	for (int i = 0; i<HEIGHT+BORDER; i++){
		for (int j = 0; j<WIDTH+BORDER; j++){
			if(init[i][j] == 0){
				label[i][j] = 3; 
				phi[i][j] = 3;
			}
			else{
				label[i][j] = -3; 
				phi[i][j] = -3;
			}
		}
	}
	for (int i = 1; i<HEIGHT+1; i++){
		for (int j = 1; j<WIDTH+1; j++){
			if(init[i][j] == 1 && checkMaskNeighbours(i, j, 1, 0) == true){
				lz.push_back(Pixel(i,j));
				label[i][j] = 0;
				phi[i][j] = 0;
			}
		}
	}
	for (it = lz.begin(); it != lz.end(); it++){
		setLevels(*it, 1); //add to either Lp1 or Ln1		
	}
	for (it = lp1.begin(); it != lp1.end(); it++){
		setLevels(*it, 2); //add to either Lp2
	}
	for (it = ln1.begin(); it != ln1.end(); it++){
		setLevels(*it, 2); //add to either Ln2
	}
}

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

//store output data to disk
void writeFile(BMP img, int id, int iter){
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
		
		sstm << iter << "label" <<".bmp";
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
		sstm << iter << "zero" <<".bmp";
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

int main(int argc, char *argv[]){
	//verify input
	if(!getAndVerifyInput(argc, argv)){
		system("pause");
		return 0;
	}

	//read input file
	BMP img;
	img.ReadFromFile("qq.bmp");
	readFile(img);
	
	//set seed point (can do multiple calls to set multiple seed points)
	if(fillSphere(250, 250, 10) == 0){
		system("pause");
		return 0;
	}
	
	initialization();
	
	calculateMu(); //only needed if the Chan Vese speed function is used
	
	list<Pixel>::iterator itt;
	printf("starting main loop\n");
	start = std::clock();
	for(int i=1; i<iterations; i++){
		prepareUpdates();
		updateLevelSets();
		if(i%100 == 0){
			printf("\niteration: %i\n", i);
		}
	}
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	printf("\nmain loop finished\n");
	printf("\ntime used: %f\n", duration);
	
	for(itt = lz.begin(); itt != lz.end(); itt++){
		zeroLevelSet[itt->x][itt->y] = 255;
	}
	writeFile(img, 1, iterations);
	writeFile(img, 2, iterations);
	system("pause");
}
