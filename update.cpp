#include <iostream>
#include <stdio.h>
#include <vector>
#include <exception>
#include <algorithm>
#include "main.h"
#include "update.h"
using namespace std;

extern double image[HEIGHT][WIDTH][DEPTH]; //image to be segmented
extern int init[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER]; //mask with seed points
extern double phi[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER]; //representation of the zero level set interface
extern int label[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER];//contains only integer values between -3 and 3
extern double F[HEIGHT][WIDTH][DEPTH];

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

double muOutside;
double muInside;

vector<double> minMaxList;

double minMax(Pixel p, int greaterOrLess, int checkAgainst){//returns max if greaterOrLess =">=", 
	double result;
	
	
	/*
	if(greaterOrLess == 1){
		for (int i = -1; i<2; i++){
			for (int j = -1; j<2; j++){
				for ( int k = -1; k<2; k++){
					if (i == 0 || j == 0 || k == 0){
						continue;
					}
					else{
						if(label[p.x+1][p.y] >= checkAgainst){
							minMaxList.push_back(phi[p.x+1][p.y]);
						}		
					}
				}
			}
		}
		result = *max_element(minMaxList.begin(), minMaxList.end());
	}
	*/
	
	
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
		result = *max_element(minMaxList.begin(), minMaxList.end());
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
		result = *min_element(minMaxList.begin(), minMaxList.end());
	}
	
	//printf("%f, %i, %i \n", result, greaterOrLess, checkAgainst);
	minMaxList.clear();
	return result;
	
}

void calculateMu(){
	double muTempInside = 0;
	int numInside = 0;
	double muTempOutside = 0;
	int numOutside = 0;
	double threshold = 0.4;
	for(int i = 0; i<HEIGHT; i++){
		for(int j = 0; j<WIDTH; j++){
			for (int k = 0; k<DEPTH; k++){
				if(image[i][j][k] > threshold){
					muTempInside += image[i][j][k];
					numInside++;
				}
				else if(image[i][j][k] < threshold){
					//printf("%f \n", image[i][j]);
					muTempOutside += image[i][j][k];
					numOutside++;
				}
			}
		}
	}
	muOutside = muTempOutside / numOutside;
	muInside = muTempInside / numInside;
	//muOutside = 0.29;
	//muInside = 0.52;
	printf("muInside: %f, muOutside: %f \n", muInside, muOutside); 
}

double speedFunction(int x, int y, int z){
	//printf("sp %f \n", (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2);
	return (((image[x][y][z] - muInside)*(image[x][y][z] - muInside)) - ((image[x][y][z] - muOutside)*(image[x][y][z] - muOutside)))/2; 
}

void prepareUpdates(){
	vector<Pixel>::iterator it;
	for(it = lz.begin(); it<lz.end();){//find pixels that are moving out of lz
		phi[it->x][it->y][it->z] += speedFunction(it->x, it->y, it->z);
		if(phi[it->x][it->y][it->z] > 0.5){
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
	for(it = ln1.begin(); it<ln1.end();){//find pixels that are moving out of ln1
		if(checkMaskNeighbours(it->x,it->y, it->z, 2, 0) == false){//if Ln1[i][j] has no neighbors q with label(q) == 0
			sn2.push_back(*it);
			it = ln1.erase(it);
		}
		else{
			double M = minMax(*it, 1, 0);
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
	for(it = lp1.begin(); it<lp1.end();){//find pixels that are moving out of lp1
		if(checkMaskNeighbours(it->x,it->y, it->z,  2, 0) == false){
			sp2.push_back(*it);
			it = lp1.erase(it);
		}
		else{
			double M = minMax(*it, -1, 0);
			phi[it->x][it->y][it->z] = M+1;
			if(phi[it->x][it->y][it->z] <= 0.5){ 
				sz.push_back(*it);
				it = lp1.erase(it);
			}
			else if(phi[it->x][it->y][it->z] > 1.5){
				sp2.push_back(*it);
				it = lp1.erase(it);
			}
			else{
				it++;
			}
		}
	}
	for(it = ln2.begin(); it < ln2.end();){
		if(checkMaskNeighbours(it->x, it->y, it->z, 2, -1) == false){
			label[it->x][it->y][it->z] = -3;
			phi[it->x][it->y][it->z] = -3;
			it = ln2.erase(it);
		}
		else{
			double M = minMax(*it, 1, -1);
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
	for(it = lp2.begin(); it < lp2.end();){
		if(checkMaskNeighbours(it->x, it->y, it->z, 2, 1) == false){
			label[it->x][it->y][it->z] = 3;
			phi[it->x][it->y][it->z] = 3;
			it = lp2.erase(it);
		}
		else{
			double M = minMax(*it, -1, 1);
			phi[it->x][it->y][it->z] = M+1;
			if(phi[it->x][it->y][it->z] <= 1.5){
				sp1.push_back(*it);
				it = lp2.erase(it);
			}
			else if(phi[it->x][it->y][it->z] > 2.5){
				label[it->x][it->y][it->z] = 3;
				phi[it->x][it->y][it->z] = 3;
				it = lp2.erase(it);
			}
			else{
				it++;
			}
		}
	}
}

void updateLevelSets(){	
	vector<Pixel>::iterator it;
	
	for (it = sz.begin(); it < sz.end(); it++){
		label[it->x][it->y][it->z] = 0;					
		lz.push_back(*it);		
	}
	sz.clear();

	for (it = sn1.begin(); it < sn1.end(); it++){
		label[it->x][it->y][it->z] = -1;
		ln1.push_back(*it);
		if (phi[it->x+1][it->y][it->z] == -3){	
			phi[it->x+1][it->y][it->z] = phi[it->x][it->y][it->z]-1;
			it->x++;	//[x+1,y,z]
			sn2.push_back(*it);
			it->x--;	//Siden vi bruker if, og ikke if else setninger må verdien settes tilbake til x
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
	
	for (it = sp1.begin(); it < sp1.end(); it++){
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
			sn2.push_back(*it);
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
	
	for (it = sn2.begin(); it < sn2.end(); it++){
		label[it->x][it->y][it->z] = -2;
		ln2.push_back(*it);
	}
	sn2.clear();

	for (it = sp2.begin(); it < sp2.end(); it++){
		label[it->x][it->y][it->z] = 2;
		lp2.push_back(*it);
	}
	sp2.clear();
}