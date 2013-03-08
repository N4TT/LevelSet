#include <iostream>
#include <stdio.h>
#include <vector>
#include <exception>
#include "main.h"
#include "update.h"
using namespace std;

extern double image[HEIGHT][WIDTH]; //image to be segmented
extern int init[HEIGHT+BORDER][WIDTH+BORDER]; //mask with seed points
extern double phi[HEIGHT+BORDER][WIDTH+BORDER]; //representation of the zero level set interface
extern int label[HEIGHT+BORDER][WIDTH+BORDER];//contains only integer values between -3 and 3
extern double F[HEIGHT][WIDTH];

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

int minMax(int i, int j, int mm){ //mm==1 -> max, mm ==0 -> min
	if(mm==1){
		return max(max(max(phi[i+1][j], phi[i-1][j]) , phi[i][j+1]) ,phi[i][j-1]);
	}
	else{
		return min(min(min(phi[i+1][j], phi[i-1][j]) , phi[i][j+1]) ,phi[i][j-1]);
	}
	
}

<<<<<<< HEAD
void calculateMu(){
	double muTempInside = 0;
	int numInside = 0;
	double muTempOutside = 0;
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
	printf("%f %f", muInside, muOutside); 
}

double speedFunction(int x, int y){
	printf("%f \n", (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2);
	return (((image[x][y] - muInside)*(image[x][y] - muInside)) - ((image[x][y] - muOutside)*(image[x][y] - muOutside)))/2; 
}

||||||| merged common ancestors
=======

double speedFunction(int x, int y){



}

>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
void prepareUpdates(){//har ikke forandret på denne så den støtter Pixel structs ennå
		
	vector<Pixel>::iterator it;
	
	for(it = lz.begin(); it<lz.end();){//find pixels that are moving out of lz
<<<<<<< HEAD
		phi[it->x][it->y] += speedFunction(it->x, it->y);
||||||| merged common ancestors
		phi[it->x][it->y] += F[it->x][it->y];
=======
		phi[it->x][it->y] += speedFunction(it->x,it->y);
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
		if(phi[it->x][it->y] > 0.5){
			sp1.push_back(*it);
			it = lz.erase(it);		//erases elements at index i and j
		}
		else if(phi[it->x][it->y] <= -0.5){
			sn1.push_back(*it);
			it = lz.erase(it);
		}
		else{
			it++;
		}
	}
	
	for(it = ln1.begin(); it<ln1.end();){//find pixels that are moving out of ln1
		if(checkMaskNeighbours(it->x,it->y, 2, 0) == false){//if Ln1[i][j] has no neighbors q with label(q) == 0
			sn2.push_back(*it);
			it = ln1.erase(it);
		}
		else{
			int M = minMax(it->x, it->y, 1);
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
			int M = minMax(it->x, it->y, 0);
			phi[it->x][it->y] = M+1;
			if(phi[it->x][it->y] <= 0.5){ 
				sz.push_back(*it);
				it = lp1.erase(it);
			}
			else if(phi[it->x][it->y] > 1.5){
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
			int M = minMax(it->x, it->y, 1);
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
			int M = minMax(it->x, it->y, 0);
			phi[it->x][it->y] = M+1;
			if(phi[it->x][it->y] <= 1.5){
				sp1.push_back(*it);
				it = lp2.erase(it);
			}
			else if(phi[it->x][it->y] > 2.5){
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

void updateLevelSets(){				//Procedure 3 Delvis påbegynt for å støtte Pixel struct, se kommentarer
	
	vector<Pixel>::iterator it;
	

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
			phi[it->x][it->y-1] = phi[it->x][it->y-1];
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