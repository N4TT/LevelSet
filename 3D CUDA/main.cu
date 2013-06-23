#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include "main.h"
#include "update.h"
#include "SIPLmini/SIPLmini.cpp"

//to calculate runtime
//#include <cstdio> 
//#include <ctime> 
//clock_t start;
//double duration;

using namespace std;
//using namespace SIPL;

float image[HEIGHT][WIDTH][DEPTH] = { 0 }; //input -> image to be segmented
float phi[HEIGHT][WIDTH][DEPTH] = { 0 };
int init[HEIGHT][WIDTH][DEPTH] = { 0 };
int label[HEIGHT][WIDTH][DEPTH] = { 0 };
int zeroLevelSet[HEIGHT][WIDTH][DEPTH] = { 0 }; //output
int layer[HEIGHT][WIDTH][DEPTH]; //-> see main.h for details

int iterations;
float threshold, alpha, epsilon;

float *phiD;
int *labelD;
int *layerD;
float *imageD;

__device__ float thresholdD, alphaD, epsilonD;

#define cudaCheckErrors(msg) \
    do { \
        cudaError_t __err = cudaGetLastError(); \
        if (__err != cudaSuccess) { \
            fprintf(stderr, "Fatal error: %s (%s at %s:%d)\n", \
                msg, cudaGetErrorString(__err), \
                __FILE__, __LINE__); \
            fprintf(stderr, "*** FAILED - ABORTING\n"); \
            exit(1); \
        } \
    } while (0)

//fills init with the seed points, returns 1 if success
void fillSphere(SIPL::int3 seed, int radius){
	for(int i = seed.x - radius; i<seed.x + radius; i++){
	for(int j = seed.y - radius; j<seed.y + radius; j++){
	for(int k = seed.z - radius; k<seed.z + radius; k++){
		SIPL::int3 n(i,j,k);
		if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < radius){
			init[i][j][k] = 1;
		}
	}}}
}

/* returns true if any neighbour of coordinates (i,j,j) in either
   init[][][] (id = 1) or label[][][] (id = 2) equals res */
bool checkMaskNeighbours(int i, int j, int k, short res){
	if(init[i+1][j][k] == res)
		return true;
	else if(init[i-1][j][k] == res) 
		return true;
	else if(init[i][j+1][k] == res)
		return true;
	else if(init[i][j-1][k] == res) 
		return true;
	else if(init[i][j][k+1] == res) 
		return true;
	else if(init[i][j][k-1] == res)
		return true;
	return false;
}

//add pixels to lists according to their label
void assignLabel(int i, int j, int k, int level){
	switch(level){
	case 1:
		layer[i][j][k] = 16; //add to lp1
		label[i][j][k] = level;
		phi[i][j][k] = level;
		break;
	case 2:
		layer[i][j][k] = 17; //add to lp2
		label[i][j][k] = level;
		phi[i][j][k] = level;
		break;
	case -1:
		layer[i][j][k] = 14; //add to ln1
		label[i][j][k] = level;
		phi[i][j][k] = level;
		break;
	case -2:
		layer[i][j][k] = 13; //add to ln2
		label[i][j][k] = level;
		phi[i][j][k] = level;	
		break;
	}
}

void setLevels(int i, int j, int k, int level){
	if(label[i+1][j][k] == 3){
		assignLabel(i+1, j, k, level);
	}
	if(label[i][j+1][k] == 3){
		assignLabel(i, j+1, k, level);
	}
	if(label[i-1][j][k] == 3){
		assignLabel(i-1, j, k, level);
	}
	if(label[i][j-1][k] == 3){
		assignLabel(i, j-1, k, level);
	}
	if(label[i][j][k+1] == 3){
		assignLabel(i, j, k+1, level);
	}
	if(label[i][j][k-1] == 3){
		assignLabel(i, j, k-1, level);
	}
	
	if(label[i+1][j][k] == -3){
		assignLabel(i+1, j, k, -level);
	}
	if(label[i][j+1][k] == -3){
		assignLabel(i, j+1, k, -level);
	}
	if(label[i-1][j][k] == -3){
		assignLabel(i-1, j, k, -level);
	}
	if(label[i][j-1][k] == -3){
		assignLabel(i, j-1, k, -level);
	}
	if(label[i][j][k+1] == -3){
		assignLabel(i, j, k+1, -level);
	}
	if(label[i][j][k-1] == -3){
		assignLabel(i, j, k-1, -level);
	}
}		

//initializes Ln2, Ln1, Lz, Lp1, Lp2 based on seed point(s) in init[][][]
void initialization(){
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			for (int k = 0; k<DEPTH; k++){
				if(init[i][j][k] == 0){
					label[i][j][k] = 3; 
					phi[i][j][k] = 3;
				}
				else{
					label[i][j][k] = -3; 
					phi[i][j][k] = -3;
				}
			}
		}
	}
	for (int i = 1; i<HEIGHT+1; i++){
		for (int j = 1; j<WIDTH+1; j++){
			for (int k = 0; k<DEPTH+1; k++){
				if(init[i][j][k] == 1 && checkMaskNeighbours(i, j, k, 0) == true){
					layer[i][j][k] = 15; //add to lz
					label[i][j][k] = 0;
					phi[i][j][k]= 0;
				}
			}
		}
	}
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			for (int k = 0; k<DEPTH; k++){
				if(layer[i][j][k] == 15){ //lz
					setLevels(i, j, k, 1);
				}
			}
		}
	}
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			for (int k = 0; k<DEPTH; k++){
				if(layer[i][j][k] == 16){// lp1
					setLevels(i, j, k, 2);
				}
			}
		}
	}
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			for (int k = 0; k<DEPTH; k++){
				if(layer[i][j][k] == 14){ //ln1
					setLevels(i, j, k, 2);
				}
			}
		}
	}
}

//allocate and copy data to device
void setUpDeviceArrays(){
	const size_t arrSize = size_t(HEIGHT*WIDTH*DEPTH);
	cudaMalloc((void**)&phiD, sizeof(float)*arrSize);
	cudaCheckErrors(" cudaMalloc error1");
	cudaMalloc((void**)&labelD, sizeof(int)*arrSize);
	cudaCheckErrors(" cudaMalloc error2");
	cudaMalloc((void**)&layerD, sizeof(int)*arrSize);
	cudaCheckErrors(" cudaMalloc error3");
	cudaMalloc((void**)&imageD, sizeof(float)*arrSize);
	cudaCheckErrors(" cudaMalloc error4");
	
	cudaMemcpy(phiD, phi, sizeof(float)*arrSize, cudaMemcpyHostToDevice);
	cudaCheckErrors(" cudaMemcpy error1");
	cudaMemcpy(labelD, label, sizeof(int)*arrSize, cudaMemcpyHostToDevice);
	cudaCheckErrors(" cudaMemcpy error2");
	cudaMemcpy(layerD, layer, sizeof(int)*arrSize, cudaMemcpyHostToDevice);
	cudaCheckErrors(" cudaMemcpy error3");
	cudaMemcpy(imageD, image, sizeof(float)*arrSize, cudaMemcpyHostToDevice);
	cudaCheckErrors(" cudaMemcpy error4");
	
}

	
void displayUshortVolume(SIPL::Volume<SIPL::ushort> * V){ 
	//Volume<float2> * v2 = new Volume<float2>(V->getSize());
	for(int x = 0; x < V->getWidth(); x++) {
	for(int y = 0; y < V->getHeight(); y++) {
	for(int z = 0; z < V->getDepth(); z++) {
		SIPL::int3 n(x,y,z);	
		image[x][y][z] = (int)(x+y*V->getWidth()+z*V->getWidth()*V->getHeight())/ 2000.0f;
		if(image[x][y][z] > 1.0f){
			image[x][y][z] = 1.0f;
		}
	}}}
	
}

void displayUcharVolume(SIPL::Volume<SIPL::uchar> * V){ 
	for(int x = 0; x < V->getWidth(); x++) {
	for(int y = 0; y < V->getHeight(); y++) {
	for(int z = 0; z < V->getDepth(); z++) {					
		image[x][y][z] = (float)((int)V->data[x+y*WIDTH+z*WIDTH*HEIGHT]/ 255.0f);
		
	}}}
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

__global__ void lol(int *labelD, int a){
	int kk = 0;
	for(int i = 0; i<HEIGHT; i++){
		for(int j = 0; j < WIDTH; j++){
			for(int k = 0; k<DEPTH; k++){
				if(labelD[i + WIDTH * (j + DEPTH * k)] == 0){
					kk++;
				}
	}}}
	if(a == 1){
		printf("before labelD: %i\n", kk);
	}
	if(a == 2){
		printf("after labelD: %i\n", kk);
	}
}

int main(int argc, char *argv[]){
	if(!getAndVerifyInput(argc, argv)){
		system("pause");
		return 0;
	}
	
	
	//SIPL::int3 seed(139, 69, 78);
	//SIPL::int3 seed2(173, 89, 127);
	
	//SIPL::int3 seed(110, 107, 162);
	SIPL::int3 seed(103, 118, 128);
	fillSphere(seed, 10);
	//fillSphere(seed2, 3);
	
	//SIPL::Volume<SIPL::ushort> * V = new SIPL::Volume<SIPL::ushort>("t1_kontrast.raw", 256,256,192);
	SIPL::Volume<SIPL::uchar> * V = new SIPL::Volume<SIPL::uchar>("circle_with_values_245.raw", 256, 256, 256);
	
	initialization();
	setUpDeviceArrays(); //copy over arrays to device
	setVariablesInDevice<<<1,1>>>(threshold, epsilon, alpha);
	
	//displayUshortVolume(V);
	displayUcharVolume(V);

	const dim3 BlockDim(16, 4, 4);
    dim3 GridDim;
    GridDim.x = (WIDTH + BlockDim.x - 1) / BlockDim.x;
    GridDim.y = (HEIGHT + BlockDim.y - 1) / BlockDim.y;
	GridDim.z = (DEPTH + BlockDim.z - 1) / BlockDim.z;
	cudaDeviceSynchronize();
	int cc = 0;
	for(int i = 0; i<HEIGHT; i++){
		for(int j = 0; j < WIDTH; j++){
			for(int k = 0; k<DEPTH; k++){
				if(label[i][j][k] == 0){
					cc++;
				}
	}}}
	lol<<<1,1>>>(labelD, 1);
	printf("before label %i\n", cc);
	cudaDeviceSynchronize();
	
	printf("starting main loop\n");
	//start = std::clock();
	for(int i=1; i<iterations+1; i++){
		if(i%100 == 0){
			printf("iteration: %i\n", i);
		}
		prepareUpdates1<<<GridDim, BlockDim>>>(phiD, layerD, imageD);
		prepareUpdates2<<<GridDim, BlockDim>>>(phiD, layerD);
		prepareUpdates3<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		prepareUpdates4<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		prepareUpdates5<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		updateLevelSets1<<<GridDim, BlockDim>>>(phiD, layerD, labelD);
		updateLevelSets2<<<GridDim, BlockDim>>>(layerD, labelD);
	}
	cudaDeviceSynchronize();
	
	//duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	printf("\nmain loop finished\n");
	//printf("time used: %f\n", duration);
	//copy back result from device
	cudaMemcpy(label, labelD, sizeof(int)*(size_t)(HEIGHT*WIDTH*DEPTH), cudaMemcpyDeviceToHost);
	cudaCheckErrors("cudaMemcpyDeviceToHost error");

	cudaDeviceSynchronize();
	
	
	lol<<<1,1>>>(labelD, 2);
	cudaDeviceSynchronize();
	int tt=0;
	for (int i = 0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			for (int k = 0; k<DEPTH; k++){
				if(label[i][j][k] == 0){ //lz
					zeroLevelSet[i][j][k] = 255;
					tt++;
				}
			}
		}
	}printf("after label: %i\n", tt);
	SIPL::Volume<SIPL::uchar> * v3 = new SIPL::Volume<SIPL::uchar>(V->getSize());
	for(int x = 0; x < V->getWidth(); x++) {
	for(int y = 0; y < V->getHeight(); y++) {
	for(int z = 0; z < V->getDepth(); z++) {
	SIPL::int3 n(x,y,z);	
		v3->set(n, (SIPL::uchar)zeroLevelSet[x][y][z]);
	}}}

	v3->save("resultCUDA.raw");
	printf("file stored\n");
	cudaFree(phiD);
	cudaFree(labelD);
	cudaFree(layerD);
	cudaFree(imageD);
	system("pause");
	return 0;
}
