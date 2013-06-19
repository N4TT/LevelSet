#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include "main.h"
#include "update.h" //levelset process happens here
#include "EasyBMP.h" //library for reading bmp files
#include "IO.h" //handles input and stores output
#include <cstdio> //to calculate runtime
#include <ctime>  //to calculate runtime
using namespace std;

float image[HEIGHT][WIDTH] = { 0 }; //input -> image to be segmented
float phi[HEIGHT][WIDTH] = { 0 };
int init[HEIGHT][WIDTH] = { 0 };
int label[HEIGHT][WIDTH] = { 0 };
int zeroLevelSet[HEIGHT][WIDTH] = { 0 }; //output
int layer[HEIGHT][WIDTH]; //-> see main.h for details

int iterations;
float threshold, alpha, epsilon;

//to calculate runtime
clock_t start;
double duration;

//device arrays
float *phiD;
int *labelD;
int *layerD;
float *imageD;

__device__ float thresholdD, alphaD, epsilonD;

//fills init with circular seed point, returns 1 if success
int fillSphere(int seedX, int seedY, int radius){
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

/* returns true if any neighbour of coordinates (i,j) in either
   init[][] (id = 1) or label[][] (id = 2) equals res */
bool checkMaskNeighbours(int i, int j, int res){
	if(init[i+1][j] == res)
		return true;
	else if(init[i-1][j] == res)
		return true;
	else if(init[i][j+1] == res)
		return true;
	else if(init[i][j-1] == res)
		return true;
	return false;
}

//add pixels to lists according to their label
void assignLabel(int i, int j, int level){
	switch(level){
	case 1:
		layer[i][j] = 16; //add to lp1
		label[i][j] = level;
		phi[i][j] = level;
		break;
	case 2:
		layer[i][j] = 17; //add to lp2
		label[i][j] = level;
		phi[i][j] = level;
		break;
	case -1:
		layer[i][j] = 14; //add to ln1
		label[i][j] = level;
		phi[i][j] = level;
		break;
	case -2:
		layer[i][j] = 13; //add to ln2
		label[i][j] = level;
		phi[i][j] = level;	
		break;
	}
}

void setLevels(int i, int j, int level){
	if(label[i+1][j] == 3){
		assignLabel(i+1, j, level);
	}
	if(label[i][j+1] == 3){
		assignLabel(i, j+1, level);
	}
	if(label[i-1][j] == 3){
		assignLabel(i-1, j, level);
	}
	if(label[i][j-1] == 3){
		assignLabel(i, j-1, level);
	}
	
	if(label[i+1][j] == -3){
		assignLabel(i+1, j, -level);
	}
	if(label[i][j+1] == -3){
		assignLabel(i, j+1, -level);
	}
	if(label[i-1][j] == -3){
		assignLabel(i-1, j, -level);
	}
	if(label[i][j-1] == -3){
		assignLabel(i, j-1, -level);
	}
}	

//initializes Ln2, Ln1, Lz, Lp1, Lp2 based on seed point(s)
void initialization(){
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
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
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			if(init[i][j] == 1 && checkMaskNeighbours(i, j, 0) == true){
				layer[i][j] = 15; //add to lz
				label[i][j] = 0;
				phi[i][j] = 0;
			}
		}
	}
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			if(layer[i][j] == 15){ //lz
				setLevels(i, j, 1);
			}
		}
	}
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			if(layer[i][j] == 16){// lp1
				setLevels(i, j, 2);
			}
		}
	}
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			if(layer[i][j] == 14){ //ln1
				setLevels(i, j, 2);
			}
		}
	}
}

//allocate and copy data to device
void setUpDeviceArrays(){
	int err;
	const size_t arrSize = size_t(HEIGHT*WIDTH);
	err = cudaMalloc((void**)&phiD, sizeof(float)*arrSize);
	if(err != cudaSuccess){
		printf("phiD cudaMalloc error: %d\n", err);
	}
	err = cudaMalloc((void**)&labelD, sizeof(int)*arrSize);
	if(err != cudaSuccess){
		printf("labelD cudaMalloc error: %d\n", err);
	}
	err = cudaMalloc((void**)&layerD, sizeof(int)*arrSize);
	if(err != cudaSuccess){
		printf("layerD cudaMalloc error: %d\n", err);
	}
	err = cudaMalloc((void**)&imageD, sizeof(float)*arrSize);
	if(err != cudaSuccess){
		printf("imageD cudaMalloc error: %d\n", err);
	}
	
	err = cudaMemcpy(phiD, phi, sizeof(float)*arrSize, cudaMemcpyHostToDevice);
	if(err != cudaSuccess){
		printf("phiD cudaMemcpy error: %d\n", err);
	}
	err = cudaMemcpy(labelD, label, sizeof(int)*arrSize, cudaMemcpyHostToDevice);
	if(err != cudaSuccess){
		printf("labelD cudaMemcpy error: %d\n", err);
	}
	err = cudaMemcpy(layerD, layer, sizeof(int)*arrSize, cudaMemcpyHostToDevice);
	if(err != cudaSuccess){
		printf("layerD cudaMemcpy error: %d\n", err);
	}
	err = cudaMemcpy(imageD, image, sizeof(float)*arrSize, cudaMemcpyHostToDevice);
	if(err != cudaSuccess){
		printf("imageD cudaMemcpy error: %d\n", err);
	}
}

int main(int argc, char *argv[]){	printf("1");
	if(!getAndVerifyInput(argc, argv)){
		system("pause");
		return 0;
	}

	//read file
	BMP img;
	img.ReadFromFile("q1.bmp");
	readFile(img);
	
	if(fillSphere(250, 255, 10) == 0){
		system("pause");
		return 0;
	}
	
	initialization();
	setUpDeviceArrays(); //copy over data to device
	setVariablesInDevice<<<1,1>>>(threshold, epsilon, alpha, image);
	
	const dim3 BlockDim(16,16);
    dim3 GridDim;
    GridDim.x = (WIDTH + BlockDim.x - 1) / BlockDim.x;
    GridDim.y = (HEIGHT + BlockDim.y - 1) / BlockDim.y;
	
	printf("starting main loop\n");
	start = std::clock();
	for(int i=0; i<iterations+1; i++){
		if(i%100 == 0){
			printf("iteration: %i\n", i);
		}
		prepareUpdates1<<<GridDim, BlockDim>>>(phiD, layerD, imageD);
		prepareUpdates2<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		prepareUpdates3<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		prepareUpdates4<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		updateLevelSets1<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		updateLevelSets2<<<GridDim, BlockDim>>>(layerD, labelD);
	}
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	printf("\nmain loop finished\n");
	printf("\ntime used: %f\n", duration);
	
	int err = cudaMemcpy(label, labelD, sizeof(int)*(HEIGHT)*(WIDTH), cudaMemcpyDeviceToHost);
	if(err != cudaSuccess){
		printf("cudaMemcpy error when writing to zeroLevelset: %d\n", err);
	}
	for (int i = 1; i<HEIGHT; i++){
		for (int j = 1; j<WIDTH; j++){
			if(label[i][j] == 0){ //lz
				zeroLevelSet[i][j] = 255;
			}
		}
	}
	
	writeFile(img, 1); //store label as image
	writeFile(img, 2); //store zerolevel set as image

	system("pause");
}
