#include <iostream>
#include <stdio.h>
#include <list>
#include <exception>
#include <algorithm>
#include "main.h"
#include "update.h"
#include "SIPL/Core.hpp"
//#include <cstdio> //to calculate runtime
//#include <ctime>  //to calculate runtime

using namespace std;
using namespace SIPL;

//clock_t start;
//double duration;

float muOutside;
float muInside;


list<float> minMaxList;

float minMaxRes;
//Returns either max or min (based on greaterOrLess) of the neighbours, with values less or greater than checkAgainst
float minMax(Pixel p, short greaterOrLess, short checkAgainst){
	minMaxList.push_back(checkAgainst); //because max_element and min_element returns 0 if the list is empty
	if(greaterOrLess == 1){
		if(label[p.x+1][p.y][p.z] >= checkAgainst){
			minMaxList.push_back(phi[p.x+1][p.y][p.z]);
		}
		if(label[p.x][p.y+1][p.z] >= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y+1][p.z]);
		}
		if(label[p.x][p.y][p.z+1] >= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y][p.z+1]);	
		}
		if(label[p.x-1][p.y][p.z] >= checkAgainst){
			minMaxList.push_back(phi[p.x-1][p.y][p.z]);
		}
		if(label[p.x][p.y-1][p.z] >= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y-1][p.z]);
		}
		if(label[p.x][p.y][p.z-1] >= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y][p.z-1]);	
		}
		if(minMaxList.size() == 0){
			printf("minMaxList is empty");
		}
		minMaxRes = *max_element(minMaxList.begin(), minMaxList.end());
	}
	else if(greaterOrLess == -1){
		if(label[p.x+1][p.y][p.z] <= checkAgainst){
			minMaxList.push_back(phi[p.x+1][p.y][p.z]);
		}
		if(label[p.x][p.y+1][p.z] <= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y+1][p.z]);
		}
		if(label[p.x][p.y][p.z+1] <= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y][p.z+1]);	
		}
		if(label[p.x-1][p.y][p.z] <= checkAgainst){
			minMaxList.push_back(phi[p.x-1][p.y][p.z]);
		}
		if(label[p.x][p.y-1][p.z] <= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y-1][p.z]);
		}
		if(label[p.x][p.y][p.z-1] <= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y][p.z-1]);	
		}
		if(minMaxList.size() == 0){
			printf("minMaxList is empty");
		}
		minMaxRes = *min_element(minMaxList.begin(), minMaxList.end());
	}
	minMaxList.clear();
	return minMaxRes;
}


/*
float minMaxRes;
//Returns either max or min (based on greaterOrLess) of the neighbours, with values less or greater than checkAgainst
float minMax(Pixel p, short greaterOrLess, short checkAgainst){
	minMaxRes = checkAgainst;
	if(greaterOrLess == 1){
		if(label[p.x+1][p.y][p.z] > checkAgainst){
			minMaxRes = phi[p.x+1][p.y][p.z];
		}
		if(label[p.x][p.y+1][p.z] > checkAgainst){
			minMaxRes = phi[p.x][p.y+1][p.z];		
		}
		if(label[p.x][p.y][p.z+1] > checkAgainst){
			minMaxRes = phi[p.x][p.y][p.z+1];		
		}
		if(label[p.x-1][p.y][p.z] > checkAgainst){
			minMaxRes = phi[p.x-1][p.y][p.z];
		}
		if(label[p.x][p.y-1][p.z] > checkAgainst){
			minMaxRes = phi[p.x][p.y-1][p.z];
		}
		if(label[p.x][p.y][p.z-1] > checkAgainst){
			minMaxRes = phi[p.x][p.y][p.z-1];
		}
	}
	else if(greaterOrLess == -1){
		if(label[p.x+1][p.y][p.z] < checkAgainst){
			minMaxRes = phi[p.x+1][p.y][p.z];
		}
		if(label[p.x][p.y+1][p.z] < checkAgainst){
			minMaxRes = phi[p.x][p.y+1][p.z];		
		}
		if(label[p.x][p.y][p.z+1] < checkAgainst){
			minMaxRes = phi[p.x][p.y][p.z+1];		
		}
		if(label[p.x-1][p.y][p.z] < checkAgainst){
			minMaxRes = phi[p.x-1][p.y][p.z];
		}
		if(label[p.x][p.y-1][p.z] < checkAgainst){
			minMaxRes = phi[p.x][p.y-1][p.z];
		}
		if(label[p.x][p.y][p.z-1] < checkAgainst){
			minMaxRes = phi[p.x][p.y][p.z-1];
		}
	}
	return minMaxRes;
}
*/
void calculateMu(float threshold){
	float muTempInside = 0;
	int numInside = 0;
	float muTempOutside = 0;
	int numOutside = 0;
	for(int i = 0; i<HEIGHT; i++){
	for(int j = 0; j<WIDTH; j++){
	for (int k = 0; k<DEPTH; k++){
		if(image[i][j][k] > threshold){
			muTempInside += image[i][j][k];
			numInside++;
		}
		else if(image[i][j][k] < threshold){
			muTempOutside += image[i][j][k];
			numOutside++;
		}
	}}}
	muOutside = muTempOutside / numOutside;
	muInside = muTempInside / numInside;
	if(numInside == 0){
		muInside = 1;
	}
	if(numOutside == 0){
		muOutside = 0;
	}
}

double speedFunctionChanVese(short x, short y, short z){
	return (((image[x][y][z] - muInside)*(image[x][y][z] - muInside)) - ((image[x][y][z] - muOutside)*(image[x][y][z] - muOutside)))/2; 
}

float nPlus, nMinus, curvature; //m� flyttes in i metoden hvis det skal paralelliseres
extern float maxCurvature;
extern float minCurvature;

float speedFunction(short i, short j, short k){
	
	float data = epsilon - abs(image[i][j][k] - treshold); //the data term (based on pixel intensity)
	if (data < 0){
		data/=(epsilon-1);
	}else if(data > 0){
		data/=epsilon;
	}
	D1 d1 = D1(i, j, k); //calculates the first order derivatives
	D2 d2 = D2(i, j, k); //calculates the second order derivatives
	Normal n = Normal(d1, d2); //calculates the normals
	curvature = (n.nPlusX - n.nMinusX) + (n.nPlusY - n.nMinusY) + (n.nPlusZ - n.nMinusZ); //the curvature

	float speed = -alpha*data + (1-alpha)*(curvature/8);
	if(speed > 1){
		speed = 1;
	}
	if(speed < -1){
		speed = -1;
	}
	return speed;
}

float M;
int iteration = -1;
list<Pixel>::iterator it;
void prepareUpdates(){
	iteration++;
	//ln2: [-2.5, -1.5>  ln1: [-1,5, -0.5>   lz: [-0.5, 0.5>   lp1: [0.5, 1.5>   lp2: [1.5, 2.5>
	//start = std::clock();
	for(it = lz.begin(); it != lz.end(); it++){//find pixels that are moving out of lz
		it->f = speedFunction(it->x, it->y, it->z);
	}
	for(it = lz.begin(); it != lz.end();){//find pixels that are moving out of lz
		phi[it->x][it->y][it->z] += it->f;
		if(phi[it->x][it->y][it->z] >= 0.5){
			sp1.push_back(*it);
			it = lz.erase(it);		//erases elements at index i and j
		}
		else if(phi[it->x][it->y][it->z] < -0.5){
			sn1.push_back(*it);
			it = lz.erase(it);
		}
		else{
			it++;
		}
	}
	
	for(it = ln1.begin(); it != ln1.end();){//find pixels that are moving out of ln1
		if(checkMaskNeighbours(it->x,it->y, it->z, 2, 0) == false){//if Ln1[i][j] has no neighbors q with label(q) == 0
			sn2.push_back(*it);
			it = ln1.erase(it);
		}
		else{
			M = minMax(*it, 1, 0);
			phi[it->x][it->y][it->z] = M-1;
			if(phi[it->x][it->y][it->z] >= -0.5){ //moving from ln1 to sz
				sz.push_back(*it);
				it = ln1.erase(it);
			}
			else if(phi[it->x][it->y][it->z] < -1.5){
				sn2.push_back(*it);
				it = ln1.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = lp1.begin(); it != lp1.end();){//find pixels that are moving out of lp1
		if(checkMaskNeighbours(it->x,it->y, it->z,  2, 0) == false){
			sp2.push_back(*it);
			it = lp1.erase(it);
		}
		else{
			M = minMax(*it, -1, 0);
			phi[it->x][it->y][it->z] = M+1;
			if(phi[it->x][it->y][it->z] < 0.5){ 
				sz.push_back(*it);
				it = lp1.erase(it);
			}
			else if(phi[it->x][it->y][it->z] >= 1.5){
				sp2.push_back(*it);
				it = lp1.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = ln2.begin(); it != ln2.end();){
		if(checkMaskNeighbours(it->x, it->y, it->z, 2, -1) == false){
			label[it->x][it->y][it->z] = -3;
			phi[it->x][it->y][it->z] = -3;
			it = ln2.erase(it);
		}
		else{
			M = minMax(*it, 1, -1);
			phi[it->x][it->y][it->z] = M-1;
			if(phi[it->x][it->y][it->z] >= -1.5){ 
				sn1.push_back(*it);
				it = ln2.erase(it);
			}
			else if(phi[it->x][it->y][it->z] < -2.5){
				label[it->x][it->y][it->z] = -3;
				phi[it->x][it->y][it->z] = -3;
				it = ln2.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = lp2.begin(); it != lp2.end();){
		if(checkMaskNeighbours(it->x, it->y, it->z, 2, 1) == false){
			label[it->x][it->y][it->z] = 3;
			phi[it->x][it->y][it->z] = 3;
			it = lp2.erase(it);
		}
		else{
			M = minMax(*it, -1, 1);
			phi[it->x][it->y][it->z] = M+1;
			if(phi[it->x][it->y][it->z] < 1.5){
				sp1.push_back(*it);
				it = lp2.erase(it);
			}
			else if(phi[it->x][it->y][it->z] >= 2.5){
				label[it->x][it->y][it->z] = 3;
				phi[it->x][it->y][it->z] = 3;
				it = lp2.erase(it);
			}
			else{
				it++;
			}
		}
	}
	//duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	//printf("\n Preperaupdates: %f", duration);
}

void updateLevelSets(){	
	//start = std::clock();
	
	for (it = sz.begin(); it != sz.end(); it++){
		label[it->x][it->y][it->z] = 0;					
		lz.push_back(*it);		
	}
	sz.clear();

	for (it = sn1.begin(); it != sn1.end(); it++){
		label[it->x][it->y][it->z] = -1;
		ln1.push_back(*it);
		if (phi[it->x+1][it->y][it->z] == -3){	
			phi[it->x+1][it->y][it->z] = phi[it->x][it->y][it->z]-1;
			it->x++;	//[x+1,y,z]
			sn2.push_back(*it);
			it->x--;	//Siden vi bruker if, og ikke if else setninger m� verdien settes tilbake til x
		}
		if (phi[it->x][it->y+1][it->z] == -3){		
			phi[it->x][it->y+1][it->z] = phi[it->x][it->y][it->z]-1;
			it->y++;	//[x,y+1,z]
			sn2.push_back(*it);
			it->y--;
		}
		if (phi[it->x][it->y][it->z+1] == -3){			
			phi[it->x][it->y][it->z+1] = phi[it->x][it->y][it->z]-1;
			it->z++;	//[x,y,z+1]
			sn2.push_back(*it);
			it->z--;
		}
		if (phi[it->x-1][it->y][it->z] == -3){			
			phi[it->x-1][it->y][it->z] = phi[it->x][it->y][it->z]-1;
			it->x--;	//[x-1,y,z]
			sn2.push_back(*it);
			it->x++;
		}
		if (phi[it->x][it->y-1][it->z] == -3){			
			phi[it->x][it->y-1][it->z] = phi[it->x][it->y][it->z]-1;
			it->y--;	//[x,y-1,z]
			sn2.push_back(*it);
			it->y++;
		}
		if (phi[it->x][it->y][it->z-1] == -3){			
			phi[it->x][it->y][it->z-1] = phi[it->x][it->y][it->z]-1;
			it->z--;	//[x,y,z-1]
			sn2.push_back(*it);
			it->z++;
		}
	}
	sn1.clear();
	
	for (it = sp1.begin(); it != sp1.end(); it++){
		label[it->x][it->y][it->z] = 1;
		lp1.push_back(*it);
		if (phi[it->x+1][it->y][it->z] == 3){			
			phi[it->x+1][it->y][it->z] = phi[it->x][it->y][it->z]+1;
			it->x++;
			sp2.push_back(*it);
			it->x--;
		}
		if (phi[it->x][it->y+1][it->z] == 3){	
			phi[it->x][it->y+1][it->z] = phi[it->x][it->y][it->z]+1;
			it->y++;
			sp2.push_back(*it);
			it->y--;
		}
		if (phi[it->x][it->y][it->z+1] == 3){			
			phi[it->x][it->y][it->z+1] = phi[it->x][it->y][it->z]+1;
			it->z++;	//[x,y,z-1]
			sp2.push_back(*it); //her l� feilen, det stod sn2.push_back istedenfor sp2.push_back
			it->z--;
		}
		if (phi[it->x-1][it->y][it->z] == 3){			
			phi[it->x-1][it->y][it->z] = phi[it->x][it->y][it->z]+1;
			it->x--;
			sp2.push_back(*it);
			it->x++;
		}
		if (phi[it->x][it->y-1][it->z] == 3){			
			phi[it->x][it->y-1][it->z] = phi[it->x][it->y][it->z]+1;
			it->y--;
			sp2.push_back(*it);
			it->y++;
		}
		if (phi[it->x][it->y][it->z-1] == 3){			
			phi[it->x][it->y][it->z-1] = phi[it->x][it->y][it->z]+1;
			it->z--;	//[x,y,z-1]
			sp2.push_back(*it);
			it->z++;
		}
	}
	sp1.clear();
	
	for (it = sn2.begin(); it != sn2.end(); it++){
		label[it->x][it->y][it->z] = -2;
		ln2.push_back(*it);
	}
	sn2.clear();

	for (it = sp2.begin(); it != sp2.end(); it++){
		label[it->x][it->y][it->z] = 2;
		lp2.push_back(*it);
	}
	sp2.clear();
	//printf("\n");
	//duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	//printf("\n updateslevelset: %f", duration);
}