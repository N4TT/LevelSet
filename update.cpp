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

int minMax(int i, int j, int mm){ //mm==1 -> max, mm ==0 -> min
	if(mm==1){
		return max(max(max(phi[i+1][j], phi[i-1][j]) , phi[i][j+1]) ,phi[i][j-1]);
	}
	else{
		return min(min(min(phi[i+1][j], phi[i-1][j]) , phi[i][j+1]) ,phi[i][j-1]);
	}
	
}

void prepareUpdates(){//har ikke forandret på denne så den støtter Pixel structs ennå
		
	vector<Pixel>::iterator it;
	
	for(int i =0; i<lz.size(); i+=2){//find pixels that are moving out of lz
		int j = i+1;
		phi[lz[i]][lz[j]] += F[lz[i]][lz[j]];
		if(phi[lz[i]][lz[j]] > 0.5){
			sp1.push_back(lz[i]);
			sp1.push_back(lz[j]);
			lz.erase(lz.begin() +i, lz.begin() +j);		//erases elements at index i and j
		}
		else if(phi[lz[i]][lz[j]] <= -0.5){
			sn1.push_back(lz[i]);
			sn1.push_back(lz[j]);
			lz.erase(lz.begin() +i, lz.begin() +j);
		}
	}
	for(int i =0; i<ln1.size(); i+=2){//find pixels that are moving out of ln1
		int j = i+1;
		if(checkMaskNeighbours(lz[i],lz[j],2, 0) == false){//if Ln1[i][j] has no neighbors q with label(q) == 0
			sn2.push_back(lz[i]);
			sn2.push_back(lz[j]);
			ln1.erase(ln1.begin() +i, ln1.begin() +j);
		}
		else{
			int M = minMax(ln1[i], ln1[j], 1);
			phi[ln1[i]][ln1[j]] = M-1;
			if(phi[ln1[i]][ln1[j]] >= -0.5){ //moving from ln1 to sz
				sz.push_back(ln1[i]);
				sz.push_back(ln1[j]);
				ln1.erase(ln1.begin() +i, ln1.begin() +j);
			}
			else if(phi[ln1[i]][ln1[j]] < -1.5){
				sn2.push_back(ln1[i]);
				sn2.push_back(ln1[j]);
				ln1.erase(ln1.begin() + i, ln1.begin() + j);
			}
		}
	}
	for(int i =0; i<lp1.size(); i+=2){//find pixels that are moving out of lp1
		int j = i+1;
		if(checkMaskNeighbours(lp1[i],lp1[j], 2, 0) == false){
			sp2.push_back(lp1[i]);
			sp2.push_back(lp1[j]);
			lp1.erase(lp1.begin() +i, lp1.begin() +j);
		}
		else{
			int M = minMax(lp1[i], lp1[j], 0);
			phi[lp1[i]][lp1[j]] = M+1;
			if(phi[lp1[i]][lp1[j]] <= 0.5){ 
				sz.push_back(lp1[i]);
				sz.push_back(lp1[j]);
				lp1.erase(lp1.begin() +i, lp1.begin() +j);
			}
			else if(phi[lp1[i]][lp1[j]] > 1.5){
				sp2.push_back(lp1[i]);
				sp2.push_back(lp1[j]);
				lp1.erase(lp1.begin() + i, lp1.begin() + j);
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