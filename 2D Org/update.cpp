#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>
#include <exception>
#include <algorithm>
#include "main.h"
#include "update.h"
using namespace std;

float muOutside;
float muInside;

vector<float> minMaxList;

float minMax(Pixel p, short greaterOrLess, short checkAgainst){//returns max if greaterOrLess =">=", 
	float result;
	if(greaterOrLess == 1){
		if(label[p.x+1][p.y] >= checkAgainst){
			minMaxList.push_back(phi[p.x+1][p.y]);
		}
		if(label[p.x][p.y+1] >= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y+1]);
		}
		if(label[p.x-1][p.y] >= checkAgainst){
			minMaxList.push_back(phi[p.x-1][p.y]);
		}
		if(label[p.x][p.y-1] >= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y-1]);
		}
		if(minMaxList.size() == 0){
			printf("minMaxList is empty");
		}
		result = *max_element(minMaxList.begin(), minMaxList.end());
	}
	else if(greaterOrLess == -1){
		if(label[p.x+1][p.y] <= checkAgainst){
			minMaxList.push_back(phi[p.x+1][p.y]);
		}
		if(label[p.x][p.y+1] <= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y+1]);
		}
		if(label[p.x-1][p.y] <= checkAgainst){
			minMaxList.push_back(phi[p.x-1][p.y]);
		}
		if(label[p.x][p.y-1] <= checkAgainst){
			minMaxList.push_back(phi[p.x][p.y-1]);
		}
		if(minMaxList.size() == 0){
			printf("minMaxList is empty");
		}
		result = *min_element(minMaxList.begin(), minMaxList.end());
	}
	//printf("%f, %i, %i \n", result, greaterOrLess, checkAgainst);
	minMaxList.clear();
	return result;
	
}

void calculateMu(){
	float muTempInside = 0;
	int numInside = 0;
	float muTempOutside = 0;
	int numOutside = 0;
	double threshold = 0.5;
	for(int i = 0; i<HEIGHT; i++){
		for(int j = 0; j<WIDTH; j++){
			if(image[i][j] > threshold){
				muTempInside += image[i][j];
				numInside++;
			}
			else if(image[i][j] < threshold){
				//printf("%f \n", image[i][j]);
				muTempOutside += image[i][j];
				numOutside++;
			}
		}
	}
	muOutside = muTempOutside / numOutside;
	muInside = muTempInside / numInside;
	//muOutside = 0.29;
	//muInside = 0.52;
	printf("muInside: %f, muOutside: %f \n", muInside, muOutside); 
}

float speedFunctionOld(int x, int y){
	//printf("sp: %f \n", (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2);
	return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2; 
}


float nPlus, nMinus; //må flyttes in i metoden hvis det skal paralelliseres
float deltaPhi, curvature;
void dPhiMin(D1 d1){
float dPhiX, dPhiY;
	dPhiX = pow(max(d1.dxPlus,0.0f),2.0f) + pow(max(-d1.dxMinus,0.0f),2.0f);
	dPhiY = pow(max(d1.dyPlus,0.0f),2.0f) + pow(max(-d1.dyMinus,0.0f),2.0f);
	deltaPhi = sqrt(dPhiX + dPhiY);
}

void dPhiMax(D1 d1){
float dPhiX, dPhiY;
	dPhiX = pow(min(d1.dxPlus,0.0f),2.0f) + pow(min(-d1.dxMinus,0.0f),2.0f);
	dPhiY = pow(min(d1.dyPlus,0.0f),2.0f) + pow(min(-d1.dyMinus,0.0f),2.0f);
	deltaPhi = sqrt(dPhiX + dPhiY);  //istede for å ta roten av dPhiX og dPhiY for så å opphøye de i andre før de legges sammen i denne linjen,
}          //skipper vi roten og legger de heller kun sammen når vi skal finne lengden på vektoren

float speedFunction(short i, short j){
	float data = epsilon - abs(image[i][j] - treshold); //the data term (based on pixel intensity)
	D1 d1 = D1(i, j); //calculates the first order derivatives
	D2 d2 = D2(i, j); //calculates the second order derivatives
	Normal n = Normal(d1, d2); //calculates the normals
	curvature = (n.nPlusX - n.nMinusX) + (n.nPlusY - n.nMinusY); //the curvature
	float F = 1*alpha*data + (1-alpha)*curvature; //kanskje det første leddet ikke skal ganges med -1
	
	if (F<0){
		dPhiMin(d1);
	}
	else{
		dPhiMax(d1);
	}
	float ret = F;//*deltaPhi;
	//printf("speed: %f\n", ret);
	return ret; 
}

vector<Pixel>::iterator it;
float M = 0;
void prepareUpdates(){
	//printf("\nsize prepare: %i ", sz.size());
	for(it = lz.begin(); it<lz.end(); it++){//find pixels that are moving out of lz
		it->f = speedFunction(it->x, it->y);
	}
	for(it = lz.begin(); it<lz.end();){//find pixels that are moving out of lz
		phi[it->x][it->y] += it->f;
		if(phi[it->x][it->y] >= 0.5){
			sp1.push_back(*it);
			//printf("\nsize prepare: %i ", lz.size());
			it = lz.erase(it);		//erases elements at index i and j
		}
		else if(phi[it->x][it->y] < -0.5){
			sn1.push_back(*it);
			it = lz.erase(it);
		}
		else{
			//printf("\nphi[%i][%i] = %f ",it->x, it->y, phi[it->x][it->y]);
			it++;
		}
	}
	
	for(it = ln1.begin(); it<ln1.end();){//find pixels that are moving out of ln1
		if(checkMaskNeighbours(it->x,it->y, 2, 0) == false){//if Ln1[i][j] has no neighbors q with label(q) == 0
			sn2.push_back(*it);
			it = ln1.erase(it);
		}
		else{
			M = minMax(*it, 1, 0);
			phi[it->x][it->y] = M-1;
			if(phi[it->x][it->y] >= -0.5){ //moving from ln1 to sz
				sz.push_back(*it);
				it = ln1.erase(it);
			}
			else if(phi[it->x][it->y] < -1.5){
				sn2.push_back(*it);
				it = ln1.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = lp1.begin(); it<lp1.end();){//find pixels that are moving out of lp1
		if(checkMaskNeighbours(it->x,it->y, 2, 0) == false){
			sp2.push_back(*it);
			it = lp1.erase(it);
		}
		else{
			M = minMax(*it, -1, 0);
			phi[it->x][it->y] = M+1;
			if(phi[it->x][it->y] < 0.5){ 
				sz.push_back(*it);
				it = lp1.erase(it);
			}
			else if(phi[it->x][it->y] >= 1.5){
				sp2.push_back(*it);
				it = lp1.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = ln2.begin(); it < ln2.end();){
		if(checkMaskNeighbours(it->x,it->y, 2, -1) == false){
			label[it->x][it->y] = -3;
			phi[it->x][it->y] = -3;
			it = ln2.erase(it);
		}
		else{
			M = minMax(*it, 1, -1);
			phi[it->x][it->y] = M-1;
			if(phi[it->x][it->y] >= -1.5){ 
				sn1.push_back(*it);
				it = ln2.erase(it);
			}
			else if(phi[it->x][it->y] < -2.5){
				label[it->x][it->y] = -3;
				phi[it->x][it->y] = -3;
				it = ln2.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = lp2.begin(); it < lp2.end();){
		if(checkMaskNeighbours(it->x,it->y, 2, 1) == false){
			label[it->x][it->y] = 3;
			phi[it->x][it->y] = 3;
			it = lp2.erase(it);
		}
		else{
			M = minMax(*it, -1, 1);
			phi[it->x][it->y] = M+1;
			if(phi[it->x][it->y] < 1.5){
				sp1.push_back(*it);
				it = lp2.erase(it);
			}
			else if(phi[it->x][it->y] >= 2.5){
				label[it->x][it->y] = 3;
				phi[it->x][it->y] = 3;
				it = lp2.erase(it);
			}
			else{
				it++;
			}
		}
	}
	//printf("   size prepareeee: %i ", sz.size());
}

void updateLevelSets(){	
	for (it = sz.begin(); it < sz.end(); it++){
		label[it->x][it->y] = 0;
		lz.push_back(*it);	
	}
	sz.clear();

	for (it = sn1.begin(); it < sn1.end(); it++){
		label[it->x][it->y] = -1;
		ln1.push_back(*it);
		if (phi[it->x+1][it->y] == -3){	
			phi[it->x+1][it->y] = phi[it->x][it->y]-1;
			it->x++;								//[x+1,y]
			sn2.push_back(*it);
			it->x--;								//Siden vi bruker if, og ikke if else setninger må verdien settes tilbake til x
		}
		if (phi[it->x][it->y+1] == -3){		
			phi[it->x][it->y+1] = phi[it->x][it->y]-1;
			it->y++;								//[x,y+1]
			sn2.push_back(*it);
			it->y--;
		}
		if (phi[it->x-1][it->y] == -3){			
			phi[it->x-1][it->y] = phi[it->x][it->y]-1;
			it->x--;								//[x-1,y]
			sn2.push_back(*it);
			it->x++;
		}
		if (phi[it->x][it->y-1] == -3){			
			phi[it->x][it->y-1] = phi[it->x][it->y]-1;
			it->y--;								//[x,y-1]
			sn2.push_back(*it);
			it->y++;
		}
	}
	sn1.clear();
	
	for (it = sp1.begin(); it < sp1.end(); it++){
		label[it->x][it->y] = 1;
		lp1.push_back(*it);
		if (phi[it->x+1][it->y] == 3){			
			phi[it->x+1][it->y] = phi[it->x][it->y]+1;
			it->x++;									//[x+1,y]
			sp2.push_back(*it);
			it->x--;
		}
		if (phi[it->x][it->y+1] == 3){	
			phi[it->x][it->y+1] = phi[it->x][it->y]+1;
			it->y++;									//[x,y+1]
			sp2.push_back(*it);
			it->y--;
		}
		if (phi[it->x-1][it->y] == 3){			
			phi[it->x-1][it->y] = phi[it->x][it->y]+1;
			it->x--;
			sp2.push_back(*it);
			it->x++;
		}
		if (phi[it->x][it->y-1] == 3){			
			phi[it->x][it->y-1] = phi[it->x][it->y]+1;
			it->y--;
			sp2.push_back(*it);
			it->y++;
		}
	}
	sp1.clear();
	
	for (it = sn2.begin(); it < sn2.end(); it++){
		label[it->x][it->y] = -2;
		ln2.push_back(*it);
	}
	sn2.clear();

	for (it = sp2.begin(); it < sp2.end(); it++){
		label[it->x][it->y] = 2;
		lp2.push_back(*it);
	}
	sp2.clear();
}