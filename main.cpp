#include <gtk/gtk.h>
#include <iostream>
#include <stdio.h>
#include <list>
#include <exception>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "update.h"
#include "SIPL/Core.hpp"

#include <cstdio> //to calculate runtime
#include <ctime>  //to calculate runtime
clock_t start;
double duration;
using namespace std;
using namespace SIPL;

float image[HEIGHT][WIDTH][DEPTH] = {0}; //input -> data to be segmented
float phi[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = {0};  //level set
short init[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = {0}; //binary mask with seed points
short label[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = {0}; //contains info about the layers
short zeroLevelSet[HEIGHT][WIDTH][DEPTH] = {0}; //output
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

void fillSphere(int3 seed, int radius){
	for(int i = seed.x - radius; i<seed.x + radius; i++){
	for(int j = seed.y - radius; j<seed.y + radius; j++){
	for(int k = seed.z - radius; k<seed.z + radius; k++){
		int3 n(i,j,k);
		if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < radius){
			init[i][j][k] = 1;
		}
	}}}
}

bool checkMaskNeighbours(int i, int j, int k, int id, short res){
	if(id == 1){ //id == 1 -> init
		if(init[i+1][j][k] == res) //right neighbour
			return true;
		else if(init[i-1][j][k] == res) //left neighbour
			return true;
		else if(init[i][j+1][k] == res) //neighbour over
			return true;
		else if(init[i][j-1][k] == res) //neighbour under
			return true;
		else if(init[i][j][k+1] == res) //neighbour in front
			return true;
		else if(init[i][j][k-1] == res) //neighbour behind
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

void pushAndStuff(Pixel p, short level){
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

void setLevels(Pixel p, short level){
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
		pushAndStuff(Pixel(p.x+1, p.y, p.z), -level);
	}
	if(label[p.x][p.y+1][p.z] == -3){
		pushAndStuff(Pixel(p.x, p.y+1, p.z), -level);
	}
	if(label[p.x-1][p.y][p.z] == -3){
		pushAndStuff(Pixel(p.x-1, p.y, p.z), -level);
	}
	if(label[p.x][p.y-1][p.z] == -3){
		pushAndStuff(Pixel(p.x, p.y-1, p.z), -level);
	}
	if(label[p.x][p.y][p.z+1] == -3){
		pushAndStuff(Pixel(p.x, p.y, p.z+1), -level);
	}
	if(label[p.x][p.y][p.z-1] == -3){
		pushAndStuff(Pixel(p.x, p.y, p.z-1), -level);
	}

}	

void initialization(){
	list<Pixel>::iterator it;

	for (int i = 0; i<HEIGHT+BORDER; i++){
		for (int j = 0; j<WIDTH+BORDER; j++){
			for (int k = 0; k<DEPTH+BORDER; k++){
				if(init[i][j][k] == 0){
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
				if(init[i][j][k] == 1 && checkMaskNeighbours(i, j, k, 1, 0) == true){
					lz.push_back(Pixel(i,j,k));
					label[i][j][k] = 0;
					phi[i][j][k]= 0;
				}
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







void displayUcharVolume(Volume<uchar> * V, int3 seed, int display){ 
	//display == 0 -> display the input mhd data, display == 1 -> display the segmentation result
	Volume<float2> * v2 = new Volume<float2>(V->getSize());
	if(display == 0){ //display input data
		for(int x = 0; x < V->getWidth(); x++) {
		for(int y = 0; y < V->getHeight(); y++) {
		for(int z = 0; z < V->getDepth(); z++) {
			float2 list;
			list.x = (float)V->get(x,y,z) / 255.0f;
			int3 n(x,y,z);	
			image[x][y][z] = (int)V->get(x,y,z) / 255.0f;
			if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
				list.y = 1.0f;
			}
			v2->set(n, list);
		}}}
		//v2->showMIP();
		v2->show();
	}
	else{ //display segmentation result
		for(int x = 0; x < V->getWidth(); x++) {
		for(int y = 0; y < V->getHeight(); y++) {
		for(int z = 0; z < V->getDepth(); z++) {
			float2 list;
			list.x = (float)zeroLevelSet[x][y][z] / 255.0f;
			int3 n(x,y,z);	
			if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
			//if(seed.distance(n) < 5.0f) {
				list.y = 1.0f;
			}
			v2->set(n, list);
		}}}
		//v2->showMIP();
		v2->show();
	}
}


void displayShortVolume(Volume<short> * V, int3 seed, int display){ 
	//display == 0 -> display the input mhd data, display == 1 -> display the segmentation result
	Volume<float2> * v2 = new Volume<float2>(V->getSize());
	if(display == 0){ //display input data
		for(int x = 0; x < V->getWidth(); x++) {
		for(int y = 0; y < V->getHeight(); y++) {
		for(int z = 0; z < V->getDepth(); z++) {
			float2 list;
			list.x = ((float)V->get(x,y,z) + 1000.0f)/ 4000.0f;		//even tho the input can have any value allowed by short, its known to be between -1000 and 1000
			int3 n(x,y,z);	
			image[x][y][z] = ((int)V->get(x,y,z) + 1000.0f)/ 4000.0f;
			if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
				list.y = 1.0f;
			}
			v2->set(n, list);
		}}}
		//v2->showMIP();
		v2->show();
	}
	else{ //display segmentation result
		for(int x = 0; x < V->getWidth(); x++) {
		for(int y = 0; y < V->getHeight(); y++) {
		for(int z = 0; z < V->getDepth(); z++) {
			float2 list;
			list.x = (float)zeroLevelSet[x][y][z] / 255.0f;
			int3 n(x,y,z);	
			if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
			//if(seed.distance(n) < 5.0f) {
				list.y = 1.0f;
			}
			v2->set(n, list);
		}}}
		//v2->showMIP();
		v2->show();
	}
}


void displayUshortVolume(Volume<ushort> * V, int3 seed, int3 seed2, int display){ 
	//display == 0 -> display the input mhd data, display == 1 -> display the segmentation result
	Volume<float2> * v2 = new Volume<float2>(V->getSize());
	if(display == 0){ //display input data
		for(int x = 0; x < V->getWidth(); x++) {
		for(int y = 0; y < V->getHeight(); y++) {
		for(int z = 0; z < V->getDepth(); z++) {
			float2 list;
			list.x = (float)V->get(x,y,z) / 2000.0f;		
			int3 n(x,y,z);	
			image[x][y][z] = (int)V->get(x,y,z) / 2000.0f;
			if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
				list.y = 1.0f;
			}
			if(sqrt((float)((seed2.x-n.x)*(seed2.x-n.x)+(seed2.y-n.y)*(seed2.y-n.y)+(seed2.z-n.z)*(seed2.z-n.z))) < 5.0f){
				list.y = 1.0f;
			}
			v2->set(n, list);
		}}}
		//v2->showMIP();
		v2->show();
	}
	else{ //display segmentation result
		for(int x = 0; x < V->getWidth(); x++) {
		for(int y = 0; y < V->getHeight(); y++) {
		for(int z = 0; z < V->getDepth(); z++) {
			float2 list;
			list.x = (float)zeroLevelSet[x][y][z] / 255.0f;
			int3 n(x,y,z);	
			if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 5.0f){
			//if(seed.distance(n) < 5.0f) {
				list.y = 1.0f;
			}
			
			if(sqrt((float)((seed2.x-n.x)*(seed2.x-n.x)+(seed2.y-n.y)*(seed2.y-n.y)+(seed2.z-n.z)*(seed2.z-n.z))) < 5.0f){
				list.y = 1.0f;
			}

			v2->set(n, list);
		}}}
		//v2->showMIP();
		v2->show();
	}
}



int main(){
	printf("ohlol123456\n");
	
	Volume<ushort> * V = new Volume<ushort>("C:/Users/N4TT/Documents/Visual Studio 2012/Projects/LevelSetProject/LevelSetProject/new_t1_kontrast.raw", 256, 256, 192);
	//Volume<uchar> * V = new Volume<uchar>("C:/Users/N4TT/Documents/Visual Studio 2012/Projects/LevelSetProject/LevelSet/aneurism.mhd");
	//Volume<short> * V = new Volume<short>("C:/Users/N4TT/Documents/Visual Studio 2012/Projects/LevelSetProject/LevelSetProject/normalized_abdomen.raw",512,512,253);
	//Volume<ushort> * V = new Volume<ushort>("C:/Users/N4TT/Dropbox/Master stuff/LevelSet data/Pas01/191211-MRT1-Liver.mhd");
	//Volume<uchar> * V = new Volume<uchar>("rectangle.mhd");
	
	int3 seed(139, 69, 78); int3 seed2(173, 89, 127);		//ventrikkel data fra mail fra nesha
	//int3 seed2(196,82,49); int3 seed(76, 122, 45);		//funker bra med LIVER CT dataen
	//int3 seed(110, 107, 162);		//seed posisjon for aneurism bildet
	//int3 seed(189, 97, 51);		//regner med dette er et godt seed point for abdomen volumet
	displayUshortVolume(V, seed, seed2, 0); //display input
	fillSphere(seed2, 5);
	fillSphere(seed, 5); //set seed point with radius 5
	initialization();
	//calculateMu(treshold); //only needed if Chan-Vese speed function is used

	list<Pixel>::iterator itt;
	treshold = 0.24; epsilon = 0.09; alpha = 0.65;		//Virker bra med ventrikkel bildet
	//treshold = 1.0; epsilon = 0.3; alpha = 0.75;		//Virker bra med aneurism bildet
	//treshold = 0.28; epsilon = 0.06; alpha = 0.75;		//bør virke bra med liver ct volumet
	//treshold = 0.557; epsilon = 0.05; alpha = 0.75; //bør virke bra med abdomen volumet
	printf("starting main loop\n");
	int iterations = 4000;
	start = std::clock();
	for(int i=0; i<iterations; i++){
		prepareUpdates();
		updateLevelSets();
/*		if(i%100 == 0){
			printf("\nloop %i done", i);
		}*/
	}

	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	for(itt = lz.begin(); itt != lz.end(); itt++){
		zeroLevelSet[itt->x][itt->y][itt->z] = 255; //copy the zero level set pixels to zeroLevelSet
	}
	
	printf("\nmain loop finished\n");
	printf("\n time used: %f", duration);
	
	displayUshortVolume(V, seed, seed2, 1); //display result
	
	//write and store output data
	Volume<uchar> * v3 = new Volume<uchar>(V->getSize());
	for(int x = 0; x < V->getWidth(); x++) {
	for(int y = 0; y < V->getHeight(); y++) {
	for(int z = 0; z < V->getDepth(); z++) {
	int3 n(x,y,z);	
		v3->set(x,y,z, (uchar)zeroLevelSet[x][y][z]);
	}}}
	
	//v3->save("new_brain_4000_its_t024e009a065.raw");
	printf("file stored\n");
	
	system("pause");
	return 0;
}