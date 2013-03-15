//#include "SIPL/Core.hpp"
//#include <gtk/gtk.h>
#include "EasyBMP.h"
#include <iostream>
#include <stdio.h>
#include <vector>
#include <exception>
#include <stdlib.h>
//#include <string.h>
#include "main.h"
#include "update.h"

using namespace std;

double image[HEIGHT][WIDTH] = { 0 };//{ {0.1,0.2,0.1,0.3,0.4},{0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}  };
double phi[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
int init[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
int label[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
double F[HEIGHT][WIDTH] = { 0 };
int zeroLevelSet[HEIGHT][WIDTH] = { 0 }; //output

vector<Pixel> lz;
vector<Pixel> lp1;
vector<Pixel> ln1;
vector<Pixel> lp2;
vector<Pixel> ln2;

vector<Pixel> sz;
vector<Pixel> sp1;
vector<Pixel> sn1;
vector<Pixel> sp2;
vector<Pixel> sn2;

void fillInit(int minX, int minY, int maxX, int maxY){
	if(maxX - minX <= 0){
		throw -1;
	}
	else if(maxY - minY <= 0){
		throw -1;
	}
	else if(minX < 0 || maxX > HEIGHT || minY < 0 || maxY > WIDTH){
		throw 1;
	}
	for (int i = minY+1; i<maxY+1; i++){
		for (int j = minX+1; j<maxX+1; j++){
			init[i][j] = 1;
		}
	}
}

bool checkMaskNeighbours(int i, int j, int id, int res){ //res er verdien som vi sjekker opp mot, kriteriet for success
	if(id == 1){ //id == 1 -> init
		if(init[i+1][j] == res)
			return true;
		else if(init[i-1][j] == res)
			return true;
		else if(init[i][j+1] == res)
			return true;
		else if(init[i][j-1] == res)
			return true;
	}
	else if(id == 2){ //id == 2 -> label
		if(label[i+1][j] == res)
			return true;
		else if(label[i-1][j] == res)
			return true;
		else if(label[i][j+1] == res)
			return true;
		else if(label[i][j-1] == res)
			return true;
	}
	return false;
}

void pushAndStuff(Pixel p, int level){//st�tter Pixel struct
	switch(level){
	case 1:
		lp1.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;
		break;
	case 2:
		lp2.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;
		break;
	case -1:
		ln1.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;
		break;
	case -2:
		ln2.push_back(p);
		label[p.x][p.y] = level;
		phi[p.x][p.y] = level;	
		break;
	}
}

void setLevels(Pixel p, int level){//st�tter Pixel Struct

	if(label[p.x+1][p.y] == 3){
		pushAndStuff(Pixel(p.x+1, p.y), level);
	}
	if(label[p.x][p.y+1] == 3){
		pushAndStuff(Pixel(p.x, p.y+1), level);
	}
	if(label[p.x-1][p.y] == 3){
		pushAndStuff(Pixel(p.x-1, p.y), level);
	}
	if(label[p.x][p.y-1] == 3){
		pushAndStuff(Pixel(p.x, p.y-1), level);
	}
	if(label[p.x+1][p.y] == -3){
		pushAndStuff(Pixel(p.x+1, p.y), -level);
	}
	if(label[p.x][p.y+1] == -3){
		pushAndStuff(Pixel(p.x, p.y+1), -level);
	}
	if(label[p.x-1][p.y] == -3){
		pushAndStuff(Pixel(p.x-1, p.y), -level);
	}
	if(label[p.x][p.y-1] == -3){
		pushAndStuff(Pixel(p.x, p.y-1), -level);
	}

}	

void initialization(){

	vector<Pixel>::iterator it;

	for (int i = 0; i<HEIGHT+BORDER; i++){
		for (int j = 0; j<WIDTH+BORDER; j++){
			if(init[i][j] == 0){
				label[i][j] = 3; 
				phi[i][j] = 3;
			}
			else{
				label[i][j] = -3; 
				phi[i][j] = -3;
			}
		}
	}
	for (int i = 1; i<HEIGHT+1; i++){
		for (int j = 1; j<WIDTH+1; j++){
			if(init[i][j] == 1 && checkMaskNeighbours(i, j, 1, 0) == true){
				lz.push_back(Pixel(i,j));
				label[i][j] = 0;
				phi[i][j] = 0;
			}
		}
	}
	
	for (it = lz.begin(); it < lz.end(); it++){
		setLevels(*it, 1);//second levelSet (level 1)			
	}
	
	for (it = lp1.begin(); it < lp1.end(); it++){
		setLevels(*it, 2);
	}
	for (it = ln1.begin(); it < ln1.end(); it++){
		setLevels(*it, 2);
	}
	
}

void readFile(BMP img){
	//copy input data (img) to image[][] and normalize to [0, 1]
	for (int i =0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			image[i][j] = img(i,j)->Red;
			image[i][j] /= 255;
		}
	}
	printf("image filled \n");
}

void writeFile(BMP img, int id){
	if(id == 1){ //label
		for (int i =0; i<HEIGHT; i++){
			for (int j = 0; j<WIDTH; j++){
				img(i,j)->Red = (label[i][j] +3)*42; //normalize to [0, 255]
				img(i,j)->Green = (label[i][j] +3)*42;
				img(i,j)->Blue = (label[i][j] +3)*42;
			}
		}
	}
	else if(id == 2){ //phi
		for (int i =0; i<HEIGHT; i++){
			for (int j = 0; j<WIDTH; j++){
				img(i,j)->Red = (phi[i][j] +3)*42; //normalize to [0, 255]
				img(i,j)->Green = (phi[i][j] +3)*42;
				img(i,j)->Blue = (phi[i][j] +3)*42;
			}
		}
	}
	else{ //zeroLevelSet
		for (int i =0; i<HEIGHT; i++){
			for (int j = 0; j<WIDTH; j++){
				img(i,j)->Red = zeroLevelSet[i][j]; 
				img(i,j)->Green = zeroLevelSet[i][j]; 
				img(i,j)->Blue = zeroLevelSet[i][j]; 
			}
		}
	}
	img.WriteToFile("output.bmp");
}

int main(){
	//read file
	BMP img;
	img.ReadFromFile("q1.bmp");
	readFile(img);
	
	try{
		fillInit(221, 208, 396, 269);
		printf("init filled\n");
	}catch(int e){
		if(e == -1){
			printf("minX er st�rre enn maxX eller minY er st�rre enn maxY\n");
			system("pause");

		}
		else if(e == 1){
			printf("masken kan ikke v�re utenfor eller st�rre enn bildet\n");
			system("pause");
		}
	}
	initialization();
	calculateMu();
	
	vector<Pixel>::iterator itt;

	printf("starting main loop\n");
	int iterations = 100;
	for(int i=0; i<iterations; i++){
		prepareUpdates();
		updateLevelSets();
		if(i == (iterations-1)){ //copy the zero level set pixels to zeroLevelSet
			for(itt = lz.begin(); itt<lz.end(); itt++){
				zeroLevelSet[itt->x][itt->y] = 255;
			}
		}
	}
	printf("main loop finished\n");
	
	writeFile(img, 3);
	printf("output successfully stored");
	
	system("pause");
	
}
