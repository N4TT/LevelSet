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

#define index(i,j,k) (i + WIDTH * (j + DEPTH * k))

//[ i*cols+j + rows*cols*z] ???
//[x * 256 * 256 + y * 256 + z] ???
//Flat[x + WIDTH * (y + DEPTH * z)] = Original[x, y, z] -> TEST UT DENNE FØRST

//nvcc --machine 32 -arch sm_20 dd.cpp main.cu update.cu 

void __global__ setVariablesInDevice(float threshold, float epsilon, float alpha){
	thresholdD = threshold;
	epsilonD = epsilon;
	alphaD = alpha;
}

//Returns either max or min (based on greaterOrLess) of the neighbours, with values less or greater than checkAgainst
__device__ float minMax(int i, int j, int k, int greaterOrLess, int checkAgainst, float *phiD, int *labelD){
	float minMaxRes = checkAgainst;
	if(greaterOrLess == 1){
		if(labelD[index(i+1,j,k)] >= minMaxRes){
			minMaxRes = phiD[index(i+1,j,k)];
		}
		if(labelD[index(i,j+1,k)] >= minMaxRes){
			minMaxRes = phiD[index(i,j+1,k)];
		}
		if(labelD[index(i-1,j,k)] >= minMaxRes){
			minMaxRes = phiD[index(i-1,j,k)];
		}
		if(labelD[index(i,j-1,k)] >= minMaxRes){
			minMaxRes = phiD[index(i,j-1,k)];
		}
		if(labelD[index(i,j,k+1)] >= minMaxRes){
			minMaxRes = phiD[index(i,j,k+1)];
		}
		if(labelD[index(i,j,k-1)] >= minMaxRes){
			minMaxRes = phiD[index(i,j,k-1)];
		}
	}
	else if(greaterOrLess == -1){
		if(labelD[index(i+1,j,k)] <= minMaxRes){
			minMaxRes = phiD[index(i+1,j,k)];
		}
		if(labelD[index(i,j+1,k)] <= minMaxRes){
			minMaxRes = phiD[index(i,j+1,k)];
		}
		if(labelD[index(i-1,j,k)] <= minMaxRes){
			minMaxRes = phiD[index(i-1,j,k)];
		}
		if(labelD[index(i,j-1,k)] <= minMaxRes){
			minMaxRes = phiD[index(i,j-1,k)];
		}
		if(labelD[index(i,j,k+1)] <= minMaxRes){
			minMaxRes = phiD[index(i,j,k+1)];
		}
		if(labelD[index(i,j,k-1)] <= minMaxRes){
			minMaxRes = phiD[index(i,j,k-1)];
		}
	}
	return minMaxRes;	
}

__device__ bool checkMaskNeighbours2(int i, int j, int k, int res, int *labelD){
	if(labelD[index(i+1,j,k)] == res)
		return true;
	else if(labelD[index(i-1,j,k)] == res)
		return true;
	else if(labelD[index(i,j+1,k)] == res)
		return true;
	else if(labelD[index(i,j-1,k)] == res)
		return true;
	else if(labelD[index(i,j,k+1)] == res)
		return true;
	else if(labelD[index(i,j,k-1)] == res)
		return true;
	return false;
	
}

__device__ float speedFunction(int i, int j, int k, float *phiD, float *imageD){ 
	//calculate data term
	float data = epsilonD - abs(imageD[index(i,j,k)] - thresholdD); //the data term (based on pixel intensity)
	//calculate 1-order derivatives
	float dx = (phiD[index(i+1,j,k)] - phiD[index(i-1,j,k)]) / 2;
	float dy = (phiD[index(i,j+1,k)] - phiD[index(i,j-1,k)]) / 2;
	float dz =  (phiD[index(i,j,k+1)] - phiD[index(i,j,k-1)]) / 2;
	float dxPlus  = phiD[index(i+1,j,k)] - phiD[index(i,j,k)];
	float dyPlus  = phiD[index(i,j+1,k)] - phiD[index(i,j,k)];
	float dzPlus  = phiD[index(i,j,k+1)] - phiD[index(i,j,k)];
	float dxMinus = phiD[index(i,j,k)] - phiD[index(i-1,j,k)];
	float dyMinus = phiD[index(i,j,k)] - phiD[index(i,j-1,k)];
	float dzMinus = phiD[index(i,j,k)] - phiD[index(i,j,k-1)];
	//calculate 2-order derivatives
	float dxPlusY  = (phiD[index(i+1,j+1,k)] - phiD[index(i-1,j+1,k)]) / 2;
	float dxMinusY = (phiD[index(i+1,j-1,k)] - phiD[index(i-1,j-1,k)]) / 2;
	float dxPlusZ  = (phiD[index(i+1,j,k+1)] - phiD[index(i-1,j,k+1)]) / 2;
	float dxMinusZ = (phiD[index(i+1,j,k-1)] - phiD[index(i-1,j,k-1)]) / 2;
	float dyPlusX  = (phiD[index(i+1,j+1,k)] - phiD[index(i+1,j-1,k)]) / 2;
	float dyMinusX = (phiD[index(i-1,j+1,k)] - phiD[index(i-1,j-1,k)]) / 2;
	float dyPlusZ  = (phiD[index(i,j+1,k+1)] - phiD[index(i,j-1,k+1)]) / 2;
	float dyMinusZ = (phiD[index(i,j+1,k-1)] - phiD[index(i,j-1,k-1)]) / 2;
	float dzPlusX  = (phiD[index(i+1,j,k+1)] - phiD[index(i+1,j,k-1)]) / 2;
	float dzMinusX = (phiD[index(i-1,j,k+1)] - phiD[index(i-1,j,k-1)]) / 2;
	float dzPlusY  = (phiD[index(i,j+1,k+1)] - phiD[index(i,j+1,k-1)]) / 2;
	float dzMinusY = (phiD[index(i,j-1,k+1)] - phiD[index(i,j-1,k-1)]) / 2;
	//calculate normals
	float nPlusX = dxPlus / sqrt(dxPlus * dxPlus + pow((dyPlusX + dy) / 2, 2) + pow((dzPlusX + dz) / 2, 2));
	float nPlusY = dyPlus / sqrt(dyPlus * dyPlus + pow((dxPlusY + dx) / 2, 2) + pow((dzPlusY + dz) / 2, 2));
	float nPlusZ = dzPlus / sqrt(dzPlus * dzPlus + pow((dxPlusZ + dx) / 2, 2) + pow((dyPlusZ + dy) / 2, 2));
	float nMinusX = dxMinus / sqrt(dxMinus * dxMinus + pow((dyMinusX + dy) / 2, 2) + pow((dzMinusX + dz) / 2, 2));
	float nMinusY = dyMinus / sqrt(dyMinus * dyMinus + pow((dxMinusY + dx) / 2, 2) + pow((dzMinusY + dz) / 2, 2));
	float nMinusZ = dzMinus / sqrt(dzMinus * dzMinus + pow((dxMinusZ + dx) / 2, 2) + pow((dyMinusZ + dy) / 2, 2));
	//calculate curvature
	float curvature = (nPlusX - nMinusX) + (nPlusY - nMinusY) + (nPlusZ - nMinusZ);
	//calculate the speeed
	float speed = -alphaD*data + (1-alphaD)*(curvature/8);
	if(speed > 1){
		speed = 1;
	}
	if(speed < -1){
		speed = -1;
	}
	return speed;
}


__global__ void prepareUpdates1(float *phiD, int *layerD, float *imageD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	if(layerD[index(i,j,k)] == 15){ //lz
		phiD[index(i,j,k)] += speedFunction(i,j,k, phiD, imageD);
	}	
}

__global__ void prepareUpdates2(float *phiD, int *layerD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	if(layerD[index(i,j,k)] == 15){ //lz
		if(phiD[index(i,j,k)] >= 0.5){
			layerD[index(i,j,k)] = 26; //add to sp1
		}
		else if(phiD[index(i,j,k)] < -0.5){
			layerD[index(i,j,k)] = 24; //add to sn1
		}
	}	
}

__global__ void prepareUpdates3(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		float M = 0;
		if(layerD[index(i,j,k)] == 14){ //ln1
			if(checkMaskNeighbours2(i, j, k, 0, labelD) == false){
				layerD[index(i,j,k)] = 23; //add to sn2
			}
			else{
				M = minMax(i, j, k,1, 0, phiD, labelD);
				phiD[index(i,j,k)] = M-1;
				if(phiD[index(i,j,k)] >= -0.5){
					layerD[index(i,j,k)] = 25; //add to sz
				}
				else if(phiD[index(i,j,k)] < -1.5){
					layerD[index(i,j,k)] = 23; //add to sn2
				}
			}
		}
	}
}

__global__ void prepareUpdates4(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		float M = 0;
		if(layerD[index(i,j,k)] == 16){ //lp1
			if(checkMaskNeighbours2(i, j, k, 0, labelD) == false){
				layerD[index(i,j,k)] = 27; //add to sp2
			}
			else{
				M = minMax(i, j, k,-1, 0, phiD, labelD);
				phiD[index(i,j,k)] = M+1;
				if(phiD[index(i,j,k)] < 0.5){
					layerD[index(i,j,k)] = 25; //add to sz
				}
				else if(phiD[index(i,j,k)] >= 1.5){
					layerD[index(i,j,k)] = 27; //add to sp2
				}
			}
		}
	}
}

__global__ void prepareUpdates5(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		float M = 0;
		if(layerD[index(i,j,k)] == 13){ //ln2
			if(checkMaskNeighbours2(i, j, k, -1, labelD) == false){
				labelD[index(i,j,k)] = -3;
				phiD[index(i,j,k)] = -3;
				layerD[index(i,j,k)] = 0; //no longer part of ln2
			}
			else{
				M = minMax(i, j, k, 1, -1, phiD, labelD);
				phiD[index(i,j,k)] = M-1;
				if(phiD[index(i,j,k)] >= -1.5){
					layerD[index(i,j,k)] = 24; //add to sn1
				}
				else if(phiD[index(i,j,k)] < -2.5){
					labelD[index(i,j,k)] = -3;
					phiD[index(i,j,k)] = -3;
					layerD[index(i,j,k)] = 0; //no longer part of ln2
				}
			}
		}
		
		if(layerD[index(i,j,k)] == 17){ //lp2
			if(checkMaskNeighbours2(i, j, k, 1, labelD) == false){
				labelD[index(i,j,k)] = 3;
				phiD[index(i,j,k)] = 3;
				layerD[index(i,j,k)] = 0; //no longer part of lp2
			}
			else{
				M = minMax(i, j, k,-1, 1, phiD, labelD);
				phiD[index(i,j,k)] = M+1;
				if(phiD[index(i,j,k)] < 1.5){
					layerD[index(i,j,k)] = 26; //add to sp1
				}
				else if(phiD[index(i,j,k)] >= 2.5){
					labelD[index(i,j,k)] = 3;
					phiD[index(i,j,k)] = 3;
					layerD[index(i,j,k)] = 0; //no longer part of lp2
				}
			}
		}
	}
}

__global__ void updateLevelSets1(float *phiD, int *layerD, int *labelD){
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	if(i !=0 && j !=0 && i < HEIGHT-1 && j < WIDTH-1){
		if(layerD[index(i,j,k)] == 25){ //sz
			labelD[index(i,j,k)] = 0;
			layerD[index(i,j,k)] = 15; //add to lz
		}
		if(layerD[index(i,j,k)] == 24){ //sn1
			labelD[index(i,j,k)] = -1;
			layerD[index(i,j,k)] = 14; //add to ln1
			if(phiD[index(i+1,j,k)] == -3){
				phiD[index(i+1,j,k)] = phiD[index(i,j,k)] - 1;
				layerD[index(i+1,j,k)] = 23; //add to sn2
			}
			if(phiD[index(i,j+1,k)] == -3){
				phiD[index(i,j+1,k)] = phiD[index(i,j,k)] - 1;
				layerD[index(i,j+1,k)] = 23; //add to sn2
			}
			if(phiD[index(i,j,k+1)] == -3){
				phiD[index(i,j,k+1)] = phiD[index(i,j,k)] - 1;
				layerD[index(i,j,k+1)] = 23; //add to sn2
			}
			if(phiD[index(i-1,j,k)] == -3){
				phiD[index(i-1,j,k)] = phiD[index(i,j,k)] - 1;
				layerD[index(i-1,j,k)] = 23; //add to sn2
			}
			if(phiD[index(i,j-1,k)] == -3){
				phiD[index(i,j-1,k)] = phiD[index(i,j,k)] - 1;
				layerD[index(i,j-1,k)] = 23; //add to sn2
			}
			if(phiD[index(i,j,k-1)] == -3){
				phiD[index(i,j,k-1)] = phiD[index(i,j,k)] - 1;
				layerD[index(i,j,k-1)] = 23; //add to sn2
			}
		}
		if(layerD[index(i,j,k)] == 26){ //sp1
			labelD[index(i,j,k)] = 1;
			layerD[index(i,j,k)] = 16; ////add to lp1
			if(phiD[index(i+1,j,k)] == 3){
				phiD[index(i+1,j,k)] = phiD[index(i,j,k)] + 1;
				layerD[index(i+1,j,k)] = 27; //add to sp2
			}
			if(phiD[index(i,j+1,k)] == 3){
				phiD[index(i,j+1,k)] = phiD[index(i,j,k)] + 1;
				layerD[index(i,j+1,k)] = 27; //add to sp2
			}
			if(phiD[index(i,j,k+1)] == 3){
				phiD[index(i,j,k+1)] = phiD[index(i,j,k)] + 1;
				layerD[index(i,j,k+1)] = 27; //add to sp2
			}
			if(phiD[index(i-1,j,k)] == 3){
				phiD[index(i-1,j,k)] = phiD[index(i,j,k)] + 1;
				layerD[index(i-1,j,k)] = 27; //add to sp2
			}
			if(phiD[index(i,j-1,k)] == 3){
				phiD[index(i,j-1,k)] = phiD[index(i,j,k)] + 1;
				layerD[index(i,j-1,k)] = 27; //add to sp2
			}
			if(phiD[index(i,j,k-1)] == 3){
				phiD[index(i,j,k-1)] = phiD[index(i,j,k)] + 1;
				layerD[index(i,j,k-1)] = 27; //add to sn2
			}
		}
	}
}
	
__global__ void updateLevelSets2(int *layerD, int *labelD){	
	int i = threadIdx.x + blockDim.x * blockIdx.x;
	int j = threadIdx.y + blockDim.y * blockIdx.y;
	int k = threadIdx.z + blockDim.z * blockIdx.z;
	//no need to check if i and j are within range here
	if(layerD[index(i,j,k)] == 23){ //sn2
		labelD[index(i,j,k)] = -2;
		layerD[index(i,j,k)] = 13;  //add to ln2
	}
	else if(layerD[index(i,j,k)] == 27){ //sp2
		labelD[index(i,j,k)] = 2;
		layerD[index(i,j,k)] = 17; //add to lp2
	}
}