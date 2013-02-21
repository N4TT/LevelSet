#include <iostream>
#include <stdio.h>
#include <vector>
#include <exception>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "update.h"
using namespace std;

double image[HEIGHT][WIDTH] = { {0.1,0.2,0.1,0.3,0.4},{0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}  };
double phi[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
int init[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
int label[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
double F[HEIGHT][WIDTH] = { 0 };

vector<int> lz;
vector<int> lp1;
vector<int> ln1;
vector<int> lp2;
vector<int> ln2;

vector<int> sz;
vector<int> sp1;
vector<int> sn1;
vector<int> sp2;
vector<int> sn2;

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

bool checkMaskNeighbours(int i, int j, int id, int res){
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

void pushAndStuff(int i, int j, int level){
	switch(level){
	case 1:
		lp1.push_back(i);
		lp1.push_back(j);
		label[i][j] = level;
		phi[i][j] = level;
	case 2:
		lp2.push_back(i);
		lp2.push_back(j);
		label[i][j] = level;
		phi[i][j] = level;
	case -1:
		ln1.push_back(i);
		ln1.push_back(j);
		label[i][j] = level;
		phi[i][j] = level;
	case -2:
		ln2.push_back(i);
		ln2.push_back(j);
		label[i][j] = level;
		phi[i][j] = level;	
	}
}

void setLevels(int i, int j, int level){
	if(label[i+1][j] == 3){
		pushAndStuff(i+1, j, level);
	}
	if(label[i][j+1] == 3){
		pushAndStuff(i, j+1, level);
	}
	if(label[i-1][j] == 3){
		pushAndStuff(i-1, j, level);
	}
	if(label[i][j-1] == 3){
		pushAndStuff(i, j-1, level);
	}
	if(label[i+1][j] == -3){
		pushAndStuff(i+1, j, -level);
	}
	if(label[i][j+1] == -3){
		pushAndStuff(i, j+1, -level);
	}
	if(label[i-1][j] == -3){
		pushAndStuff(i-1, j, -level);
	}
	if(label[i][j-1] == -3){
		pushAndStuff(i, j-1, -level);
	}
}	

void initialization(){

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
			if(init[i][j] == 1 && checkMaskNeighbours(i, j, 1, 0) == true)
				lz.push_back(i);		//adder i til vektoren som representerer zero level set
				lz.push_back(j);		//adder j på samme måte som i. i ligger på partall indekser mens j sitter på oddetall
				label[i][j] = 0;
				phi[i][j] = 0;

		}
	}
	for (int i = 0; i<lz.size(); i+=2){
		setLevels(lz[i], lz[i+1], 1);	//second levelSet (level 1)
	}
	for (int i = 0; i<lp1.size(); i+=2){
		setLevels(lp1[i], lp1[i+1], 2);	
	}
	for (int i = 0; i<ln1.size(); i+=2){
		setLevels(ln1[i], ln1[i+1], 2);	
	}
}






void readbmp(char* filename){

    FILE* fp = fopen(filename, "rb");

    int width, height, offset;

    fseek(fp, 18, SEEK_SET);
    fread(&width, 4, 1, fp);
    fseek(fp, 22, SEEK_SET);
    fread(&height, 4, 1, fp);
    fseek(fp, 10, SEEK_SET);
    fread(&offset, 4, 1, fp);

    unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char)*height*width);

    fseek(fp, offset, SEEK_SET);
    //We just ignore the padding :)
    fread(data, sizeof(unsigned char), height*width, fp);

    fclose(fp);

	for (int i =0; i<512; i++){
		for (int j = 0; j<512; j++){
		image[i][j] = (double)data[i*512+j]; //må kanskje forandres hvis bildet er opp ned
		
		}
	}
    //return data;
}

int main(){
	

	
	try{
		fillInit(2, 2, 3, 3);
	
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

	prepareUpdates();
	updateLevelSets();
	system("pause");

}
