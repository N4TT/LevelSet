#include "EasyBMP.h"
#include <iostream>
#include <stdio.h>
#include <list>
#include <exception>
#include <stdlib.h>
#include <sstream> //to concatenate string and int when writing multiple files in a single run
#include "main.h"
#include "update.h"

using namespace std;

float image[HEIGHT][WIDTH] = { 0 }; //input -> image to be segmented
float phi[HEIGHT+BORDER][WIDTH+BORDER] = { 0 }; //level set
short init[HEIGHT+BORDER][WIDTH+BORDER] = { 0 }; //binary mask with seed points
short label[HEIGHT+BORDER][WIDTH+BORDER] = { 0 }; //contains info about the layers
short zeroLevelSet[HEIGHT][WIDTH] = { 0 }; //output

float treshold, alpha, epsilon;

list<Pixel> lz;
list<Pixel> lp1;
list<Pixel> ln1;
list<Pixel> lp2;
list<Pixel> ln2;

list<Pixel> sz;
list<Pixel> sp1;
list<Pixel> sn1;
list<Pixel> sp2;
list<Pixel> sn2;

//fills init with the seed points, returns 1 if success
int fillSphere(int seedX, int seedY, int radius){
	if(seedX < 0 || seedX > HEIGHT || seedY < 0 || seedY > WIDTH){
		printf("Wrong input to create a circular seed\n");
		printf("Coordinates out of range\n");
		return 0;
	}
	else if(radius < 1 || radius > HEIGHT/2 || radius > WIDTH/2){
		printf("Wrong input to create a circular seed\n");
		printf("Radius must be a positive integer less than min(width, height)/2\n");
		return 0;
	}
	for(int i = seedX - radius; i < seedX + radius; i++){
		for(int j = seedY - radius; j < seedY + radius; j++){
			if(sqrt((float)((seedX-i)*(seedX-i)+(seedY-j)*(seedY-j))) < radius){
				init[i][j] = 1;
			}
		}
	}
	return 1;
}

//can replace fillSphere() if a rectangular seed point is wanted
int fillInit(short minX, short minY, short maxX, short maxY){
	if(maxX - minX <= 0 || maxY - minY <= 0){
		printf("Wrong input to create a rectangular seed\n");
		printf("Input must be in this order minX minY maxX maxY\n");
		return 0;
	}
	else if(minX < 0 || maxX >= HEIGHT || minY < 0 || maxY >= WIDTH){
		printf("Wrong input to create a rectangular seed\n");
		printf("Input out of range\n");
		return 0;
	}
	for (int i = minY+1; i<maxY+1; i++){
		for (int j = minX+1; j<maxX+1; j++){
			init[i][j] = 1;
		}
	}
	return 1;
}

bool checkMaskNeighbours(int i, int j, int id, short res){ //res er verdien som vi sjekker opp mot, kriteriet for success
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

void pushAndStuff(Pixel p, short level){//st�tter Pixel struct
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

void setLevels(Pixel p, short level){
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
	list<Pixel>::iterator it;

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
	for (it = lz.begin(); it != lz.end(); it++){
		setLevels(*it, 1);//second levelSet (level 1)			
	}
	for (it = lp1.begin(); it != lp1.end(); it++){
		setLevels(*it, 2);
	}
	for (it = ln1.begin(); it != ln1.end(); it++){
		setLevels(*it, 2);
	}
}

void readFile(BMP img){
	//copy input data (img) to image[][] and normalize to [0, 1]
	for (short i =0; i<HEIGHT; i++){
		for (short j = 0; j<WIDTH; j++){
			image[i][j] = (img(i,j)->Red + img(i,j)->Green + img(i,j)->Blue) / 3;
			image[i][j] /= 255;
		}
	}
}


void writeFile(BMP img, int id, int iter){
	string name;
	stringstream sstm;
	
	if(id == 1){ //label
		for (short i =0; i<HEIGHT; i++){
			for (short j = 0; j<WIDTH; j++){
				img(i,j)->Red = (label[i][j] +3)*42; //normalize to [0, 255]
				img(i,j)->Green = (label[i][j] +3)*42;
				img(i,j)->Blue = (label[i][j] +3)*42;
			}
		}
		sstm << "label" << iter <<".bmp";
		name = sstm.str();
		img.WriteToFile(name.c_str());
	}
	else if(id == 2){ //phi
		for (short i =0; i<HEIGHT; i++){
			for (short j = 0; j<WIDTH; j++){
				img(i,j)->Red = (phi[i][j] +3)*42; //normalize to [0, 255]
				img(i,j)->Green = (phi[i][j] +3)*42;
				img(i,j)->Blue = (phi[i][j] +3)*42;
			}
		}
		img.WriteToFile("1output phi.bmp");
	}
	else{ //zeroLevelSet
		for (short i =0; i<HEIGHT; i++){
			for (short j = 0; j<WIDTH; j++){
				img(i,j)->Red = zeroLevelSet[i][j]; 
				img(i,j)->Green = zeroLevelSet[i][j]; 
				img(i,j)->Blue = zeroLevelSet[i][j]; 
			}
		}
		sstm << "zero" << iter <<".bmp";
		name = sstm.str();
		img.WriteToFile(name.c_str());
	}
}

int main(int argc, char *argv[]){
	int iterations;
	if(argc != 2){
		printf("Need one input: number of iterations\n");
		system("pause");
		return 0;
	}

	if (sscanf (argv[1], "%i", &iterations)!=1 || iterations<1) { 
		printf("first input must be a positive integer: number of iterations\n"); 
		system("pause");
		return 0;
	}

	//read file
	BMP img;
	img.ReadFromFile("img.bmp");
	readFile(img);
	
	/*if(fillInit(150, 150, 250, 250) == 0){
		system("pause");
		return 0;
	}*/

	if(fillSphere(50, 50, 10) == 0){
		system("pause");
		return 0;
	}
	
	initialization();
	calculateMu(); //only needed if the Chan Vese speed function is used
	
	list<Pixel>::iterator itt;

	treshold = 0.99; epsilon = 0.15; alpha = 0.80;

	printf("starting main loop\n");
	
	for(int i=0; i<iterations; i++){
		prepareUpdates();
		updateLevelSets();
		/*if(i%100 == 0){
			printf("\niteration: %i\n", i);
			printf("\nwriting to zeroLevelSet");
			for(itt = lz.begin(); itt != lz.end(); itt++){
				zeroLevelSet[itt->x][itt->y] = 255;
			}
			writeFile(img, 3, i);
			writeFile(img, 1, i);
			for(itt = lz.begin(); itt != lz.end(); itt++){
				zeroLevelSet[itt->x][itt->y] = 0;
			}
		}*/
		if(i == (iterations-1)){ //copy the zero level set pixels to zeroLevelSet
			printf("\nwriting to zeroLevelSet");
			for(itt = lz.begin(); itt != lz.end(); itt++){
				zeroLevelSet[itt->x][itt->y] = 255;
			}
			writeFile(img, 3, i);
			writeFile(img, 1, i);
		}
	}
	printf("\nmain loop finished");
	
	//writeFile(img, 3, iterations);
	//writeFile(img, 1, iterations);
	printf("\noutput successfully stored\n");

	system("pause");
	
}
