
#include <gtk/gtk.h>
//#include "EasyBMP.h"
#include <iostream>
#include <stdio.h>
#include <vector>
#include <exception>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "update.h"
#include "SIPL/Core.hpp"
using namespace std;



double image[HEIGHT][WIDTH][DEPTH] = { 0 };//{ {0.1,0.2,0.1,0.3,0.4},{0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}  };
double phi[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = { 0 };
int* init = (int*) calloc(sizeof(int), (HEIGHT+BORDER)*(WIDTH+BORDER)*(DEPTH+BORDER));
//int init = new int[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER];
int label[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = { 0 };
double F[HEIGHT][WIDTH][DEPTH] = { 0 };
int zeroLevelSet[HEIGHT][WIDTH][DEPTH] = { 0 }; //output

#define index(i,j, k) ((i)+(j)*WIDTH+(k)*WIDTH*DEPTH)

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

void fillInit(int minX, int minY, int minZ, int maxX, int maxY, int maxZ){
	if(maxX - minX <= 0){
		throw -1;
	}
	else if(maxY - minY <= 0){
		throw -1;
	}
	else if(maxZ - minZ <= 0){
		throw -1;
	}
	else if(minX < 0 || maxX > HEIGHT || minY < 0 || maxY > WIDTH || minZ < 0 || maxZ > DEPTH){
		throw 1;
	}
	for (int i = minY+1; i<maxY+1; i++){
		for (int j = minX+1; j<maxX+1; j++){
			for (int k = minZ+1; k<maxZ+1; k++){
				init[index(i,j,k)] = 1; //programmet krasjer her når størrelsen er 512*512*512, men det virker fint ved 256*256*256
			}
		}
	}
	printf("\nloop done");
}

bool checkMaskNeighbours(int i, int j, int k, int id, int res){ //res er verdien som vi sjekker opp mot, kriteriet for success
	if(id == 1){ //id == 1 -> init
		if(init[index(i+1,j,k)] == res) //right neighbour
			return true;
		else if(init[index(i-1,j,k)] == res) //left neighbour
			return true;
		else if(init[index(i,j+1,k)] == res) //neighbour over
			return true;
		else if(init[index(i,j-1,k)] == res) //neighbour under
			return true;
		else if(init[index(i,j,k+1)] == res) //neighbour in front
			return true;
		else if(init[index(i,j,k)-1] == res) //neighbour behind
			return true;
	}
	else if(id == 2){ //id == 2 -> label
		if(label[i+1][j][k] == res)
			return true;
		else if(label[i-1][j][k] == res)
			return true;
		else if(label[i][j+1][k] == res)
			return true;
		else if(label[i][j-1][k] == res)
			return true;
		else if(label[i][j][k+1] == res)
			return true;
		else if(label[i][j][k-1] == res)
			return true;
	}
	return false;
}

void pushAndStuff(Pixel p, int level){//støtter Pixel struct
	switch(level){
	case 1:
		lp1.push_back(p);
		label[p.x][p.y][p.z] = level;
		phi[p.x][p.y][p.z] = level;
		break;
	case 2:
		lp2.push_back(p);
		label[p.x][p.y][p.z] = level;
		phi[p.x][p.y][p.z] = level;
		break;
	case -1:
		ln1.push_back(p);
		label[p.x][p.y][p.z] = level;
		phi[p.x][p.y][p.z] = level;
		break;
	case -2:
		ln2.push_back(p);
		label[p.x][p.y][p.z] = level;
		phi[p.x][p.y][p.z] = level;	
		break;
	}
}

void setLevels(Pixel p, int level){//støtter Pixel Struct
/*
	for(int i = p.x-1; i<p.x+1; i++){
		for(int j = p.y-1; j<p.y+1; j++){
			for(int k = p.z-1; k<p.z+1; k++){
				if(p.x != i && p.y != j &&p.z != k){
					if(label[i][j][k] == 3){
						pushAndStuff(Pixel(i, j, k), level);
					}
					else if(label[i][j][k] == -3){
						pushAndStuff(Pixel(i, j, k), level);
					}
				}
			}
		}
	}
*/
	if(label[p.x+1][p.y][p.z] == 3){
		pushAndStuff(Pixel(p.x+1, p.y, p.z), level);
	}
	if(label[p.x][p.y+1][p.z] == 3){
		pushAndStuff(Pixel(p.x, p.y+1, p.z), level);
	}
	if(label[p.x-1][p.y][p.z] == 3){
		pushAndStuff(Pixel(p.x-1, p.y, p.z), level);
	}
	if(label[p.x][p.y-1][p.z] == 3){
		pushAndStuff(Pixel(p.x, p.y-1, p.z), level);
	}
	if(label[p.x][p.y][p.z+1] == 3){
		pushAndStuff(Pixel(p.x, p.y, p.z+1), level);
	}
	if(label[p.x][p.y][p.z-1] == 3){
		pushAndStuff(Pixel(p.x, p.y, p.z-1), level);
	}

	if(label[p.x+1][p.y][p.z] == -3){
		pushAndStuff(Pixel(p.x+1, p.y, p.z), level);
	}
	if(label[p.x][p.y+1][p.z] == -3){
		pushAndStuff(Pixel(p.x, p.y+1, p.z), level);
	}
	if(label[p.x-1][p.y][p.z] == -3){
		pushAndStuff(Pixel(p.x-1, p.y, p.z), level);
	}
	if(label[p.x][p.y-1][p.z] == -3){
		pushAndStuff(Pixel(p.x, p.y-1, p.z), level);
	}
	if(label[p.x][p.y][p.z+1] == -3){
		pushAndStuff(Pixel(p.x, p.y, p.z+1), level);
	}
	if(label[p.x][p.y][p.z-1] == -3){
		pushAndStuff(Pixel(p.x, p.y, p.z-1), level);
	}

}	

void initialization(){

	vector<Pixel>::iterator it;

	for (int i = 0; i<HEIGHT+BORDER; i++){
		for (int j = 0; j<WIDTH+BORDER; j++){
			for (int k = 0; k<DEPTH+BORDER; k++){
				if(init[index(i,j,k)] == 0){
					label[i][j][k] = 3; 
					phi[i][j][k] = 3;
				}
				else{
					label[i][j][k] = -3; 
					phi[i][j][k] = -3;
				}
			}
		}
	}
	for (int i = 1; i<HEIGHT+1; i++){
		for (int j = 1; j<WIDTH+1; j++){
			for (int k = 0; k<DEPTH+1; k++){
				if(init[index(i,j,k)] == 1 && checkMaskNeighbours(i, j, k, 1, 0) == true){
					lz.push_back(Pixel(i,j,k));
					label[i][j][k] = 0;
					phi[i][j][k]= 0;
				}
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
/*
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
		printf("dsed");
		for (int i =0; i<HEIGHT; i++){
			for (int j = 0; j<WIDTH; j++){
				img(i,j)->Red = zeroLevelSet[i][j]; 
				img(i,j)->Green = zeroLevelSet[i][j]; 
				img(i,j)->Blue = zeroLevelSet[i][j]; 
			}
		}
	}
	img.WriteToFile("output.bmp");
	//img.WriteToFile("output.bmp");
}
*/

using namespace SIPL;
void displayVolume(){
	Volume<uchar> * V = new Volume<uchar>("aneurism.mhd");
	
	int3 seed(100, 100, 100);
	Volume<float2> * v2 = new Volume<float2>(V->getSize());
	for(int x = 0; x < v2->getWidth(); x++) {
	for(int y = 0; y < v2->getHeight(); y++) {
	for(int z = 0; z < v2->getDepth(); z++) {
		float2 vector;
		vector.x = (float)zeroLevelSet[x][y][z] / 255.0f;
		int3 n(x,y,z);	
		if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
		//if(seed.distance(n) < 5.0f) {
			vector.y = 1.0f;
		}
		v2->set(n, vector);
	}}}
	v2->showMIP();
	//v2->show();
	
}

int main(){

	
	Volume<uchar> * V = new Volume<uchar>("aneurism.mhd");
	//V->show();
	
	int3 seed(100, 100, 100);
	Volume<float2> * v2 = new Volume<float2>(V->getSize());
	for(int x = 0; x < v2->getWidth(); x++) {
	for(int y = 0; y < v2->getHeight(); y++) {
	for(int z = 0; z < v2->getDepth(); z++) {
		float2 vector;
		vector.x = (float)V->get(x,y,z) / 255.0f;
		int3 n(x,y,z);	
		image[x][y][z] = (int)V->get(x,y,z) / 255.0f;	
		if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
		//if(seed.distance(n) < 5.0f) {
			vector.y = 1.0f;
		}
		v2->set(n, vector);//guesswork:setter punktet n i v2 til verdiene i vector. v2 er et volum med elementer float2
	}}}						//vector.y bestemmer om punktet er innenfor radiusen til seed punktet mens vector.x er verdien til bildet i det punktet.
	//v2->showMIP();
	
	//read file
	/*
	BMP img;
	img.ReadFromFile("qq.bmp");
	readFile(img);
	*/
	/*SIPL::Init();
	SIPL::Volume<float> * v = new SIPL::Volume<float>("rawfile.raw", 181, 217, 181);
	for(int i = 50; i<180; i+=10){
		v->show(SIPL::Z, i, 80);
		//system("pause");
	}
	*/
	
	
	try{
		fillInit(100, 100, 140, 120, 120, 160);
		printf("init filled\n");
	}catch(int e){
		if(e == -1){
			printf("minX er større enn maxX eller minY er større enn maxY\n");
			system("pause");

		}
		else if(e == 1){
			printf("masken kan ikke være utenfor eller større enn bildet\n");
			system("pause");
		}
	}
	initialization();
	calculateMu();

	vector<Pixel>::iterator itt;

	printf("starting main loop\n");
	int iterations = 25;
	for(int i=0; i<iterations; i++){
		prepareUpdates();
		updateLevelSets();
		if(i == (iterations-1)){ //copy the zero level set pixels to zeroLevelSet
			for(itt = lz.begin(); itt<lz.end(); itt++){
				zeroLevelSet[itt->x][itt->y][itt->z] = 255;
			}
		}
		printf("\nloop %i done", i);
	}
	printf("\nmain loop finished\n");
	displayVolume();
	//writeFile(img, 3);
	//printf("output successfully stored");
	
	system("pause");

}