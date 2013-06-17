#include <iostream>
#include <stdio.h>
#include <cmath>
#include <cuda.h>
#include <exception>
#include <algorithm>
#include "main.h"
#include "update.h"
using namespace std;

extern float *phiD;
extern int *labelD;
extern int *layerD;
extern float *imageD;
extern __device__ float thresholdD, epsilonD, alphaD;

void __global__ setVariablesInDevice(float threshold, float epsilon, float alpha, float image[HEIGHT][WIDTH]){
	thresholdD = threshold;
	epsilonD = epsilon;
	alphaD = alpha;
}

//nvcc --machine 32 -arch sm_20 main.cu update.cu IO.cu EasyBMP.cpp 

//Returns either max or min (based on greaterOrLess) of the neighbours, with values less or greater than checkAgainst
__device__ float follow(int i, int j, int greaterOrLess, int checkAgainst, float *phiD, int *labelD){
	float fResult = checkAgainst;
	if(greaterOrLess == 1){
		if(labelD[(i+1)*WIDTH+j] >= fResult){
			fResult = phiD[(i+1)*WIDTH+j];
		}
		if(labelD[i*WIDTH+(j+1)] >= fResult){
			fResult = phiD[i*WIDTH + (j+1)];
		}
		if(labelD[(i-1)*WIDTH + j] >= fResult){
			fResult = phiD[(i-1)*WIDTH+j];
		}
		if(labelD[i*WIDTH+(j-1)] >= fResult){
			fResult = phiD[i*WIDTH+(j-1)];
		}
	}
	else if(greaterOrLess == -1){
		if(labelD[(i+1)*WIDTH+j] <= fResult){
			fResult = phiD[(i+1)*WIDTH+j];
		}
		if(labelD[i*WIDTH+(j+1)] <= fResult){
			fResult = phiD[i*WIDTH+(j+1)];
		}
		if(labelD[(i-1)*WIDTH+j] <= fResult){
			fResult = phiD[(i-1)*WIDTH+j];
		}
		if(labelD[i*WIDTH+(j-1)] <= fResult){
			fResult= phiD[i*WIDTH+(j-1)];
		}
	}
	return fResult;	
}

__device__ bool checkMaskNeighbours2(int i, int j, short res, int *labelD){
	if(labelD[(i+1)*WIDTH+j] == res)
		return true;
	else if(labelD[(i-1)*WIDTH+j] == res)
		return true;
	else if(labelD[i*WIDTH+(j+1)] == res)
		return true;
	else if(labelD[i*WIDTH+(j-1)] == res)
		return true;	
	return false;
}

__device__ float speedFunction(int i, int j, float *phiD, float *imageD){ 
	//calculate data term
	float data = epsilonD - abs(imageD[i*WIDTH+j] - thresholdD); //the data term (based on pixel intensity)
	//calculate first order derivatives
	float dx = (phiD[(i+1)*WIDTH+j] - phiD[(i-1)*WIDTH+ j]) / 2;
	float dy = (phiD[i*WIDTH+(j+1)] - phiD[i*WIDTH+(j-1)]) / 2;
	float dxPlus = phiD[(i+1)*WIDTH+j] - phiD[i*WIDTH+j];
	float dyPlus = phiD[i*WIDTH+(j+1)] - phiD[i*WIDTH+j];
	float dxMinus = phiD[i*WIDTH+j] - phiD[(i-1)*WIDTH+ j];
	float dyMinus = phiD[i*WIDTH+j] - phiD[i*WIDTH+(j-1)];
	//calculate second order derivatives
	float dxPlusY = (phiD[(i+1)*WIDTH+(j+1)] - phiD[(i-1)*WIDTH+(j+1)])/2;
	float dxMinusY = (phiD[(i+1)*WIDTH+(j-1)] - phiD[(i-1)*WIDTH+(j-1)])/2;
	float dyPlusX = (phiD[(i+1)*WIDTH+(j+1)] - phiD[(i+1)*WIDTH+(j-1)])/2;
	float dyMinusX = (phiD[(i-1)*WIDTH+(j+1)] - phiD[(i-1)*WIDTH+(j-1)])/2;
	//calculate normals
	float nPlusX = dxPlus / sqrt(dxPlus*dxPlus + pow((dyPlusX + dy) / 2, 2));
	float nPlusY = dyPlus / sqrt(dyPlus*dyPlus + pow((dxPlusY + dx) / 2, 2));
	float nMinusX = dxMinus / sqrt(dxMinus * dxMinus + pow((dyMinusX + dy) / 2, 2));
	float nMinusY = dyMinus / sqrt(dyMinus * dyMinus + pow((dxMinusY + dx) / 2, 2));
	//calculate curvature
	float curvature = (nPlusX - nMinusX) + (nPlusY - nMinusY);
	//calculate the speeed
	float speed = -alphaD*data + (1.0f-alphaD)*(curvature/4.0f); //divided by 4 to narmalize (max(curvature) = 4)
	//clamp speed
	if(speed > 1.0f){
		speed = 1.0f;
	}
	if(speed < -1.0f){
		speed = -1.0f;
	}
	return speed;
}

__global__ void prepareUpdates1(float *phiD, int *layerD, float *imageD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	if(layerD[i*WIDTH+j] == 15){ //lz
		phiD[i*WIDTH+j] += speedFunction(i,j, phiD, imageD);
		if(phiD[i*WIDTH+j] >= 0.5){
			layerD[i*WIDTH+j] = 26; //add to sp1
			
		}
		else if(phiD[i*WIDTH+j] < -0.5){
			layerD[i*WIDTH+j] = 24; //add to sn1
		}
	}
}

__global__ void prepareUpdates2(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		float M = 0;
		if(layerD[i*WIDTH+j] == 14){ //ln1
			if(checkMaskNeighbours2(i, j, 0, labelD) == false){
				layerD[i*WIDTH+j] = 23; //add to sn2
			}
			else{
				M = follow(i, j, 1, 0, phiD, labelD);
				phiD[i*WIDTH+j] = M-1;
				if(phiD[i*WIDTH+j] >= -0.5){
					layerD[i*WIDTH+j] = 25; //add to sz
				}
				else if(phiD[i*WIDTH+j] < -1.5){
					layerD[i*WIDTH+j] = 23; //add to sn2
				}
			}
		}
	}
}

__global__ void prepareUpdates3(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		float M = 0;
		if(layerD[i*WIDTH+j] == 16){ //lp1
			if(checkMaskNeighbours2(i, j, 0, labelD) == false){
				layerD[i*WIDTH+j] = 27; //add to sp2
			}
			else{
				M = follow(i, j, -1, 0, phiD, labelD);
				phiD[i*WIDTH+j] = M+1;
				if(phiD[i*WIDTH+j] < 0.5){
					layerD[i*WIDTH+j] = 25; //add to sz
				}
				else if(phiD[i*WIDTH+j] >= 1.5){
					layerD[i*WIDTH+j] = 27; //add to sp2
				}
			}
		}
	}
}

__global__ void prepareUpdates4(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		float M = 0;
		if(layerD[i*WIDTH+j] == 13){ //ln2
			if(checkMaskNeighbours2(i, j, -1, labelD) == false){
				labelD[i*WIDTH+j] = -3;
				phiD[i*WIDTH+j] = -3;
				layerD[i*WIDTH+j] = 0; //no longer part of ln2
			}
			else{
				M = follow(i, j, 1, -1, phiD, labelD);
				phiD[i*WIDTH+j] = M-1;
				if(phiD[i*WIDTH+j] >= -1.5){
					layerD[i*WIDTH+j] = 24; //add to sn1
				}
				else if(phiD[i*WIDTH+j] < -2.5){
					labelD[i*WIDTH+j] = -3;
					phiD[i*WIDTH+j] = -3;
					layerD[i*WIDTH+j] = 0; //no longer part of ln2
				}
			}
		}
		
		if(layerD[i*WIDTH+j] == 17){ //lp2
			if(checkMaskNeighbours2(i, j, 1, labelD) == false){
				labelD[i*WIDTH+j] = 3;
				phiD[i*WIDTH+j] = 3;
				layerD[i*WIDTH+j] = 0; //no longer part of lp2
			}
			else{
				M = follow(i, j, -1, 1, phiD, labelD);
				phiD[i*WIDTH+j] = M+1;
				if(phiD[i*WIDTH+j] < 1.5){
					layerD[i*WIDTH+j] = 26; //add to sp1
				}
				else if(phiD[i*WIDTH+j] >= 2.5){
					labelD[i*WIDTH+j] = 3;
					phiD[i*WIDTH+j] = 3;
					layerD[i*WIDTH+j] = 0; //no longer part of lp2
				}
			}
		}
	}
}

__global__ void updateLevelSets1(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		if(layerD[i*WIDTH+j] == 25){ //sz
			labelD[i*WIDTH+j] = 0;
			layerD[i*WIDTH+j] = 15; //add to lz
		}
		if(layerD[i*WIDTH+j] == 24){ //sn1
			labelD[i*WIDTH+j] = -1;
			layerD[i*WIDTH+j] = 14; //add to ln1
			if(phiD[(i+1)*WIDTH+ j] == -3){
				phiD[(i+1)*WIDTH+ j] = phiD[i*WIDTH+j] - 1;
				layerD[(i+1)*WIDTH+ j] = 23; //add to sn2
			}
			if(phiD[i*WIDTH+ (j+1)] == -3){
				phiD[i*WIDTH+ (j+1)] = phiD[i*WIDTH+j] - 1;
				layerD[i*WIDTH+ (j+1)] = 23; //add to sn2
			}
			if(phiD[(i-1)*WIDTH+ j] == -3){
				phiD[(i-1)*WIDTH+ j] = phiD[i*WIDTH+j] - 1;
				layerD[(i-1)*WIDTH+ j] = 23; //add to sn2
			}
			if(phiD[i*WIDTH+ (j-1)] == -3){
				phiD[i*WIDTH+ (j-1)] = phiD[i*WIDTH+j] - 1;
				layerD[i*WIDTH+ (j-1)] = 23; //add to sn2
			}
		}
		if(layerD[i*WIDTH+j] == 26){ //sp1
			labelD[i*WIDTH+j] = 1;
			layerD[i*WIDTH+j] = 16; ////add to lp1
			if(phiD[(i+1)*WIDTH+ j] == 3){
				phiD[(i+1)*WIDTH+ j] = phiD[i*WIDTH+j] + 1;
				layerD[(i+1)*WIDTH+ j] = 27; //add to sp2
			}
			if(phiD[i*WIDTH+ (j+1)] == 3){
				phiD[i*WIDTH+ (j+1)] = phiD[i*WIDTH+j] + 1;
				layerD[i*WIDTH+ (j+1)] = 27; //add to sp2
			}
			if(phiD[(i-1)*WIDTH+ j] == 3){
				phiD[(i-1)*WIDTH+ j] = phiD[i*WIDTH+j] + 1;
				layerD[(i-1)*WIDTH+ j] = 27; //add to sp2
			}
			if(phiD[i*WIDTH+ (j-1)] == 3){
				phiD[i*WIDTH+ (j-1)] = phiD[i*WIDTH+j] + 1;
				layerD[i*WIDTH+ (j-1)] = 27; //add to sp2
			}
		}
	}
}
	
__global__ void updateLevelSets2(int *layerD, int *labelD){	
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	//no need to check if i and j are within range here
	if(layerD[i*WIDTH+j] == 23){ //sn2
		labelD[i*WIDTH+j] = -2;
		layerD[i*WIDTH+j] = 13;  //add to ln2
	}
	if(layerD[i*WIDTH+j] == 27){ //sp2
		labelD[i*WIDTH+j] = 2;
		layerD[i*WIDTH+j] = 17; //add to lp2
	}
}