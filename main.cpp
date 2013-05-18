#include <gtk/gtk.h>
#include <iostream>
#include <stdio.h>
#include <list>
#include <exception>
#include <stdlib.h>
#include <string.h>
#include <sstream>
#include "main.h"
#include "update.h"
#include "SIPL/Core.hpp"

#include <cstdio> //to calculate runtime
#include <ctime>  //to calculate runtime
clock_t start;
double duration;

//#include <omp.h> //openMP

using namespace std;
using namespace SIPL;

float maxCurvature = 0;
float minCurvature = 0;
float image[HEIGHT][WIDTH][DEPTH] = { 0 };//{ {0.1,0.2,0.1,0.3,0.4},{0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}  };
float phi[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = { 0 };
//short* init = (short*) calloc(sizeof(short), (HEIGHT+BORDER)*(WIDTH+BORDER)*(DEPTH+BORDER));
short init[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = { 0 };
short label[HEIGHT+BORDER][WIDTH+BORDER][DEPTH+BORDER] = { 0 };
float F[HEIGHT][WIDTH][DEPTH] = { 0 };
short zeroLevelSet[HEIGHT][WIDTH][DEPTH] = { 0 }; //output
int num_threads;
float treshold, alpha, epsilon;

//#define index(i,j, k) ((i)+(j)*WIDTH+(k)*WIDTH*DEPTH)

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

void fillInit(short minX, short minY, short minZ, short maxX, short maxY, short maxZ){
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
				init[i][j][k] = 1;
				//init[index(i,j,k)] = 1; //programmet krasjer her når størrelsen er 512*512*512, men det virker fint ved 256*256*256
			}
		}
	}
	printf("\nloop done");
}

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

bool checkMaskNeighbours(int i, int j, int k, int id, short res){ //res er verdien som vi sjekker opp mot, kriteriet for success
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

void pushAndStuff(Pixel p, short level){//støtter Pixel struct
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

void setLevels(Pixel p, short level){//støtter Pixel Struct
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

void displayVolume(Volume<ushort> * V, int3 seed, int display){ //display == 0 -> display the input mhd data, display == 1 -> display the segmentation result
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
			v2->set(n, list);//guesswork:setter punktet n i v2 til verdiene i list. v2 er et volum med elementer float2
		}}}						//list.y bestemmer om punktet er innenfor radiusen til seed punktet mens list.x er verdien til bildet i det punktet.
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

int main(){
	//int num_threads = 10;
	//omp_set_num_threads(num_threads);
	
	
	//Volume<uchar> * V = new Volume<uchar>("aneurism.mhd");
	Volume<ushort> * V = new Volume<ushort>("Liver.mhd");
	//Volume<uchar> * V = new Volume<uchar>("circle_with_values_245.mhd");
	//Volume<uchar> * V = new Volume<uchar>("rectangle.mhd");
	
	int3 seed(189, 97, 51);
	displayVolume(V, seed, 0);
	
	try{
		//fillInit(110, 90, 140, 125, 105, 155);
		fillSphere(seed, 5);
		printf("init filled %f\n", treshold);
	}
	catch(int e){
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
	calculateMu(treshold);

	list<Pixel>::iterator itt;
	//treshold = 0.95; epsilon = 0.05; alpha = 0.95; //virker perfekt med sirkel volumet
	treshold = 1.0; epsilon = 0.3; alpha = 0.75;
	printf("starting main loop\n");
	int iterations = 500;
	start = std::clock();
	for(int i=0; i<iterations; i++){
		prepareUpdates();
		updateLevelSets();
		
		if(i == (iterations-1)){ //copy the zero level set pixels to zeroLevelSet
			for(itt = lz.begin(); itt != lz.end(); itt++){
				zeroLevelSet[itt->x][itt->y][itt->z] = 255;
			}
		}
		if(i%10 == 0){
			printf("\nloop %i done", i);
		}
	}
	duration = ( std::clock() - start ) / (double) CLOCKS_PER_SEC;
	printf("\n time used: %f", duration);
	printf("\nmain loop finished\n");
	
	displayVolume(V, seed, 1);
	
	Volume<uchar> * v3 = new Volume<uchar>(V->getSize());
	for(int x = 0; x < V->getWidth(); x++) {
	for(int y = 0; y < V->getHeight(); y++) {
	for(int z = 0; z < V->getDepth(); z++) {
	int3 n(x,y,z);	
		/*//created circle_with_values_245.raw:
		if(sqrt((float)((seed.x-n.x)*(seed.x-n.x)+(seed.y-n.y)*(seed.y-n.y)+(seed.z-n.z)*(seed.z-n.z))) < 50.0f){
			v3->set(x,y,z, (uchar)245);
		}*/
		/*//reated rectangle.raw:
		if((x > seed.x-20 && x < seed.x+20) && (y > seed.y-20 && y < seed.y+20) && (z > seed.z-20 && z < seed.z+20)){
			init[x][y][z] = 1;
				v3->set(x,y,z, (uchar)245);
		}*/
		/*//star.raw must include bmp.h and call readfromfile()
		if(z > seed.z-20 && z < seed.z+20){
			float asdf = (img(x,y)->Red + img(x,y)->Green + img(x,y)->Blue) / 3 - 1;
			v3->set(x,y,z, (uchar)asdf); -> code failed
		}*/	
		v3->set(x,y,z, (uchar)zeroLevelSet[x][y][z]);
	}}}
	printf("\n maxCurvature: %f,  minCurvature: %f \n",maxCurvature, minCurvature);
	printf("%i, %i, %i, %i, %i\n", lz.size(), ln1.size(), lp1.size(), ln2.size(), lp2.size());
	v3->save("1000iters.raw");
	printf("file stored\n");
	
	system("pause");
}