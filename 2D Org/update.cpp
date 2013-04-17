#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>
#include <exception>
#include <algorithm>
#include "main.h"
#include "update.h"
using namespace std;

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

//temp values
extern vector<Pixel> sz; //values in sz are to be moved to lz
extern vector<Pixel> sp1;
extern vector<Pixel> sn1;
extern vector<Pixel> sp2;
extern vector<Pixel> sn2;

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
	double threshold = 0.4;
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
	printf("sp: %f \n", (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2);
	return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2; 
}

/*
pixel neighbors[8];
int neighborNum = 0;
void clearNeighbors(){
	for(int i = 0; i<8; i++){
		neighbors[i] = Pixel(9,9); //9 is just a temp value
	}
}
*/
float result;
float SumLp1Neighbours;
int NumLp1Neighbours;
float SumLn1Neighbours;
int NumLn1Neighbours;
//float weights[9] = {0};// 0->right/up, 1->right, 2->right/down, 3->up, 4->middle, 5->down, 6->left/up, 7->left, 8->left/down
float speedFunction(int x, int y){
	//clearNeighbors();
	//return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2;
	SumLp1Neighbours = 0;
	NumLp1Neighbours = 0;
	
	SumLn1Neighbours = 0;
	NumLn1Neighbours = 0;
	for(int i = x-1; i<=x+1; i++){
		for(int j = y-1; j<=y+1; j++){
			if(i != x && j != y){
				if (label[i][j] == 1){ 	//if its not evaluating its own position and the neighbour is part of the zerolvlset
					SumLp1Neighbours += image[i][j];
					NumLp1Neighbours++ ;
					//printf("\nif");
				}
				else if (label[i][j] == -1){
					SumLn1Neighbours += image[i][j];
					NumLn1Neighbours++;
					//printf("\nelse");
				}	
			}
		}
	}

		result = abs(SumLp1Neighbours/NumLp1Neighbours - SumLn1Neighbours/NumLn1Neighbours);
	if( result < 0.5){
		return -(0.5-result);
	}
	else if(result > 0.5){
		return (result-0.5);
	}
	else{
		return 0;
	}
	/*
	if(NumLp1Neighbours != 0 && NumLn1Neighbours != 0){
		//printf("%f \n", 0.5 -(1 - abs(SumLp1Neighbours/NumLp1Neighbours - SumLn1Neighbours/NumLn1Neighbours)));
		//return 1.5 -(1 - abs(SumLp1Neighbours/NumLp1Neighbours - SumLn1Neighbours/NumLn1Neighbours));
		//return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2;
	}
	else{
		//printf("kfsøef\n");
		//return 0;
	}
	return rand()-0.5;
	//return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2;
	//printf("\n sp: %f", 0.5 - (1 - abs((SumLp1Neighbours/NumLp1Neighbours) - (SumLn1Neighbours/NumLn1Neighbours))));
	//printf("%f \n",(((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2);
	//return 0.5 - (1 - abs((SumLp1Neighbours/NumLp1Neighbours) - (SumLn1Neighbours/NumLn1Neighbours)));
	*/
//  -1,-1		0,-1  	1,-1
//  -1,0		0,0	 	 1,0
//   -1,1		0,1	     1,1
 
}

void prepareUpdates(){
	//printf("\nsize prepare: %i ", sz.size());
	vector<Pixel>::iterator it;
	for(it = lz.begin(); it<lz.end();){//find pixels that are moving out of lz
		phi[it->x][it->y] += speedFunction(it->x, it->y);
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
			float M = minMax(*it, 1, 0);
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
			float M = minMax(*it, -1, 0);
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
			float M = minMax(*it, 1, -1);
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
			float M = minMax(*it, -1, 1);
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
	//printf("\nsize update: %i ", sz.size());
	vector<Pixel>::iterator it;
	
	for (it = sz.begin(); it < sz.end(); it++){
		label[it->x][it->y] = 0;		
		//printf("\nsize update: %i ", lz.size());		
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
	//printf("    size update: %i \n ", sz.size());
}