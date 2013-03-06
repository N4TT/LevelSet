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

void prepareUpdates(){//har ikke forandret p� denne s� den st�tter Pixel structs enn�
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
	for(int i =0; i<ln2.size(); i+=2){
		int j = i+1;
		if(checkMaskNeighbours(ln2[i],ln2[j], 2, -1) == false){
			label[ln2[i]][ln2[j]] = -3;
			phi[ln2[i]][ln2[j]] = -3;
			ln2.erase(ln2.begin() +i, ln2.begin() +j);
		}
		else{
			int M = minMax(ln2[i], ln2[j], 1);
			phi[ln2[i]][ln2[j]] = M-1;
			if(phi[ln2[i]][ln2[j]] >= -1.5){ 
				sn1.push_back(ln2[i]);
				sn1.push_back(ln2[j]);
				ln2.erase(ln2.begin() +i, ln2.begin() +j);
			}
			else if(phi[ln2[i]][ln2[j]] < -2.5){
				label[ln2[i]][ln2[j]] = -3;
				phi[ln2[i]][ln2[j]] = -3;
				ln2.erase(ln2.begin() + i, ln2.begin() + j);
			}
		}
	}
	for(int i =0; i<lp2.size(); i+=2){
		int j = i+1;
		if(checkMaskNeighbours(lp2[i],lp2[j], 2, 1) == false){
			label[lp2[i]][lp2[j]] = 3;
			phi[lp2[i]][lp2[j]] = 3;
			lp2.erase(lp2.begin() +i, lp2.begin() +j);
		}
		else{
			int M = minMax(lp2[i], lp2[j], 0);
			phi[lp2[i]][lp2[j]] = M+1;
			if(phi[lp2[i]][lp2[j]] <= 1.5){
				sp1.push_back(lp2[i]);
				sp1.push_back(lp2[j]);
				lp2.erase(lp2.begin() +i, lp2.begin() +j);
			}
			else if(phi[lp2[i]][lp2[j]] > 2.5){
				label[lp2[i]][lp2[j]] = 3;
				phi[lp2[i]][lp2[j]] = 3;
				lp2.erase(lp2.begin() +i, lp2.begin() +j);
			}
		}
	}
}

void updateLevelSets(){				//Procedure 3 Delvis p�begynt for � st�tte Pixel struct, se kommentarer
	
	vector<Pixel>::iterator it;
	

	for (it = lz.begin(); it < sz.end(); it++){
		label[it->x][it->y] = 0;					
		lz.push_back(*it);		
		sz.erase(it);						
	}


	//HAR IKKE FIKSET FOR PIXEL STRUCT NEDENFOR YET
	for (int i = 0; i<sn1.size(); i+=2){
		int j = i+1;
		label[sn1[i]][sn1[j]] = -1;
		ln1.push_back(sn1[i]);
		ln1.push_back(sn1[j]);
		sn1.erase(sn1.begin() +i, sn1.begin() +j);
		if (phi[sn1[i]+1][sn1[j]] == -3){			//denne kan muligens legges sammen med if setningene under i en metode
			phi[sn1[i]+1][sn1[j]] = phi[sn1[i]][sn1[j]]-1;
			sn2.push_back(sn1[i]+1);
			sn2.push_back(sn1[j]);
		}
		if (phi[sn1[i]][sn1[j]+1] == -3){			
			phi[sn1[i]][sn1[j]+1] = phi[sn1[i]][sn1[j]]-1;
			sn2.push_back(sn1[i]);
			sn2.push_back(sn1[j]+1);
		}
		if (phi[sn1[i]-1][sn1[j]] == -3){			
			phi[sn1[i]-1][sn1[j]] = phi[sn1[i]][sn1[j]]-1;
			sn2.push_back(sn1[i]-1);
			sn2.push_back(sn1[j]);
		}
		if (phi[sn1[i]][sn1[j]-1] == -3){			
			phi[sn1[i]][sn1[j]-1] = phi[sn1[i]][sn1[j]]-1;
			sn2.push_back(sn1[i]);
			sn2.push_back(sn1[j]-1);
		}
	}
	printf(" 2\n");
	for (int i = 0; i<sp1.size(); i+=2){
		int j = i+1;
		label[sp1[i]][sp1[j]] = 1;
		lp1.push_back(sp1[i]);
		lp1.push_back(sp1[j]);
		sp1.erase(sp1.begin() +i, sp1.begin() +j);
		if (phi[sp1[i]+1][sp1[j]] == 3){			//denne kan muligens legges sammen med if setningene under i en metode
			phi[sp1[i]+1][sp1[j]] = phi[sp1[i]][sp1[j]]+1;
			sp2.push_back(sp1[i]+1);
			sp2.push_back(sp1[j]);
		}
		if (phi[sp1[i]][sp1[j]+1] == 3){	
			phi[sp1[i]][sp1[j]+1] = phi[sp1[i]][sp1[j]]+1;
			sp2.push_back(sp1[i]);
			sp2.push_back(sp1[j]+1);
		}
		if (phi[sp1[i]-1][sp1[j]] == 3){			
			phi[sp1[i]-1][sp1[j]] = phi[sp1[i]][sp1[j]]+1;
			sp2.push_back(sp1[i]-1);
			sp2.push_back(sp1[j]);
		}
		if (phi[sp1[i]][sp1[j]-1] == 3){			
			phi[sp1[i]][sp1[j]-1] = phi[sp1[i]][sp1[j]]+1;
			sp2.push_back(sp1[i]);
			sp2.push_back(sp1[j]-1);
		}
	}
	printf(" 3\n");
	for (int i = 0; i<sn2.size(); i+=2){
		int j = i+1;
		label[sn2[i]][sn2[j]] = -2;
		ln2.push_back(sn2[i]);
		ln2.push_back(sn2[j]);
		sn2.erase(sn2.begin()+i, sn2.begin()+j);
	}
	printf(" 4\n");
	for (int i = 0; i<sp2.size(); i+=2){
		int j = i+1;
		label[sp2[i]][sp2[j]] = 2;
		lp2.push_back(sp2[i]);
		lp2.push_back(sp2[j]);
		sp2.erase(sp2.begin()+i, sp2.begin()+j);
	}
}