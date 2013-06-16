#include <iostream>
#include <stdio.h>
#include <cmath>
#include <list>
#include <exception>
#include <algorithm>
#include "main.h"
#include "update.h"
using namespace std;

/*average values of back- and foreground,
only used if the Chan-Vese speedfunciton is used*/
float muOutside; 
float muInside;

float minMaxRes;
//Returns either max or min (based on greaterOrLess) of the neighbours, with values less or greater than checkAgainst
float minMax(Pixel p, short greaterOrLess, short checkAgainst){
	minMaxRes = checkAgainst;
	if(greaterOrLess == 1){
		if(label[p.x+1][p.y] >= minMaxRes){
			minMaxRes = phi[p.x+1][p.y];
		}
		if(label[p.x][p.y+1] >= minMaxRes){
			minMaxRes = phi[p.x][p.y+1];
		}
		if(label[p.x-1][p.y] >= minMaxRes){
			minMaxRes = phi[p.x-1][p.y];
		}
		if(label[p.x][p.y-1] >= minMaxRes){
			minMaxRes = phi[p.x][p.y-1];
		}
	}
	else if(greaterOrLess == -1){
		if(label[p.x+1][p.y] <= minMaxRes){
			minMaxRes = phi[p.x+1][p.y];
		}
		if(label[p.x][p.y+1] <= minMaxRes){
			minMaxRes = phi[p.x][p.y+1];
		}
		if(label[p.x-1][p.y] <= minMaxRes){
			minMaxRes = phi[p.x-1][p.y];
		}
		if(label[p.x][p.y-1] <= minMaxRes){
			minMaxRes = phi[p.x][p.y-1];
		}
	}
	return minMaxRes;
}

/*calculates the average back- and foreground values
only used if the Chan-Vese speedfunciton is used*/
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
				muTempOutside += image[i][j];
				numOutside++;
			}
		}
	}
	muOutside = muTempOutside / numOutside;
	muInside = muTempInside / numInside;
	if(numInside == 0){
		muInside = 1;
	}
	if(numOutside == 0){
		muOutside = 0;
	}
}

float speedFunctionChanVese(int x, int y){
	return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2; 
}

float curvature;
float speedFunction(short i, short j){
	float data = epsilon - abs(image[i][j] - threshold); //the data term (based on pixel intensity)
	D1 d1 = D1(i, j); //calculates the first order derivatives
	D2 d2 = D2(i, j); //calculates the second order derivatives
	Normal n = Normal(d1, d2); //calculates the normals
	curvature = (n.nPlusX - n.nMinusX) + (n.nPlusY - n.nMinusY); //the curvature
	float speed = -alpha*data + (1-alpha)*(curvature/4);
	if(speed > 1){
		speed = 1;
	}
	if(speed < -1){
		speed = -1;
	}
	return speed;
}

list<Pixel>::iterator it;
float M = 0;
void prepareUpdates(){
	for(it = lz.begin(); it != lz.end();){ //Lz
		phi[it->x][it->y] += speedFunction(it->x, it->y);
		if(phi[it->x][it->y] >= 0.5){
			sp1.push_back(*it);
			it = lz.erase(it);
		}
		else if(phi[it->x][it->y] < -0.5){
			sn1.push_back(*it);
			it = lz.erase(it);
		}
		else{
			it++;
		}
	}
	
	for(it = ln1.begin(); it != ln1.end();){ //pixels moving out of Ln1
		if(checkMaskNeighbours(it->x,it->y, 2, 0) == false){
		//if none of the neighbors are in Lz
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
	for(it = lp1.begin(); it != lp1.end();){ //pixels moving out of Lp1
		if(checkMaskNeighbours(it->x,it->y, 2, 0) == false){
		//if none of the neighbors are in Lz
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
	for(it = ln2.begin(); it != ln2.end();){
		if(checkMaskNeighbours(it->x,it->y, 2, -1) == false){
		//if none of the neighbors are in Ln1
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
	for(it = lp2.begin(); it != lp2.end();){
		if(checkMaskNeighbours(it->x,it->y, 2, 1) == false){
		//if none of the neighbors are in Lp1
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
}

void updateLevelSets(){	
	for (it = sz.begin(); it != sz.end(); it++){
		label[it->x][it->y] = 0;
		lz.push_back(*it);	
	}
	sz.clear();

	for (it = sn1.begin(); it != sn1.end(); it++){
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
	
	for (it = sp1.begin(); it != sp1.end(); it++){
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
	
	for (it = sn2.begin(); it != sn2.end(); it++){
		label[it->x][it->y] = -2;
		ln2.push_back(*it);
	}
	sn2.clear();

	for (it = sp2.begin(); it != sp2.end(); it++){
		label[it->x][it->y] = 2;
		lp2.push_back(*it);
	}
	sp2.clear();
}