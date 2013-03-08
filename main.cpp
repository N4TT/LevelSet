//#include "SIPL/Core.hpp"
//#include <gtk/gtk.h>
#include <iostream>
#include <stdio.h>
#include <vector>
#include <exception>
#include <stdlib.h>
#include <string.h>
#include "main.h"
#include "update.h"
#include "bmp.h"
using namespace std;

double image[HEIGHT][WIDTH] = { 0 };//{ {0.1,0.2,0.1,0.3,0.4},{0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}, {0.1,0.2,0.1,0.3,0.4}  };
double phi[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
int init[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
int label[HEIGHT+BORDER][WIDTH+BORDER] = { 0 };
double F[HEIGHT][WIDTH] = { 0 };



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

void pushAndStuff(Pixel p, int level){//støtter Pixel struct
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
<<<<<<< HEAD
		phi[p.x][p.y] = level;	
		break;
||||||| merged common ancestors
		phi[p.x][p.y] = level;	
=======
		phi[p.x][p.y] = level;
		break;
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
	}
}

void setLevels(Pixel p, int level){//støtter Pixel Struct

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

void initialization(){//støtter Pixel struct, men se comment under dersom du feilsøker

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

void readbmp(char* filename){

    FILE* fp = fopen(filename, "rb");

    int width, height, offset;

    fseek(fp, 18, SEEK_SET);
    fread(&width, 4, 1, fp);
    fseek(fp, 22, SEEK_SET);
    fread(&height, 4, 1, fp);
    fseek(fp, 10, SEEK_SET);
    fread(&offset, 4, 1, fp);
	printf("the height is: %i\n", height);
    unsigned char* data = (unsigned char*)malloc(sizeof(unsigned char)*height*width);
<<<<<<< HEAD
	
	
||||||| merged common ancestors

=======
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
    fseek(fp, offset, SEEK_SET);
    //We just ignore the padding :)
    fread(data, sizeof(unsigned char), height*width, fp);

    fclose(fp);
<<<<<<< HEAD
||||||| merged common ancestors
	//printf("dara: %c", data[0]);
=======
	//printf("data: %c", data[0]);
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
	for (int i =0; i<height; i++){
		for (int j = 0; j<width; j++){
<<<<<<< HEAD
			image[i][j] = ((double)data[i*height+j])/255; //må kanskje forandres hvis bildet er opp ned
||||||| merged common ancestors
		image[i*height][j] = (double)data[i*height+j]; //må kanskje forandres hvis bildet er opp ned
		
=======
		image[i][j] = (double)data[i*height+j]; //må kanskje forandres hvis bildet er opp ned
		
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
		}
	}
}

void write_bmp(unsigned char* data, int width, int height){
    struct bmp_id id;
    id.magic1 = 0x42;
    id.magic2 = 0x4D;

    struct bmp_header header;
    header.file_size = width*height+54 + 2;
    header.pixel_offset = 1078;

    struct bmp_dib_header dib_header;
    dib_header.header_size = 40;
    dib_header.width = width;
    dib_header.height = height;
    dib_header.num_planes = 1;
    dib_header.bit_pr_pixel = 8;
    dib_header.compress_type = 0;
    dib_header.data_size = width*height;
    dib_header.hres = 0;
    dib_header.vres = 0;
    dib_header.num_colors = 256;
    dib_header.num_imp_colors = 0;

    char padding[2];

    unsigned char* color_table = (unsigned char*)malloc(1024);
    for(int c= 0; c < 256; c++){
		color_table[c*4] = (unsigned char) (c*33.3+140); //green
        color_table[c*4+1] = (unsigned char) (c*33.3+140); //red
        color_table[c*4+2] = 0; //opacity?
        color_table[c*4+3] = (unsigned char) (c*33.3+140); //blue
    }

    FILE* fp = fopen("out.bmp", "w+");
    fwrite((void*)&id, 1, 2, fp);
    fwrite((void*)&header, 1, 12, fp);
    fwrite((void*)&dib_header, 1, 40, fp);
    fwrite((void*)color_table, 1, 1024, fp);
    fwrite((void*)data, 1, width*height, fp);
    fwrite((void*)&padding,1,2,fp);
    fclose(fp);
}


int main(){//Ikke rørt etter Pixel struct ble opprettet
	
<<<<<<< HEAD
	readbmp("img.bmp");
||||||| merged common ancestors
	//readbmp("img.bmp");
	//printf("reading done\n");
=======
	readbmp("img.bmp");
	printf("reading done\n");
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
	

	try{
		fillInit(225, 225, 300, 300);
		printf("filling done\n");
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
	printf("starting main loop\n");
<<<<<<< HEAD
	
	calculateMu();
	for(int i = 0; i<3; i++){
||||||| merged common ancestors
	/*
	for(int i = 0; i<5; i++){
=======
	
	for(int i = 0; i<50; i++){
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
		prepareUpdates();
		updateLevelSets();
<<<<<<< HEAD
	}
	printf("looping done");
||||||| merged common ancestors
		printf("update done\n");
	}
	*/
	/*for (int i = 0; i<lp1.size(); i++){
		if (lp1[i] > 513)
			printf(" lp1: %d ", lp1[i]);
	}
	for (int i = 0; i<lp2.size(); i++){
		if (lp2[i] > 513)
			printf(" lp2: %d ", lp2[i]);
	}
	for (int i = 0; i<ln1.size(); i++){
		if (ln1[i] > 513)
			printf(" ln1: %d", ln1[i]);
	}
	for (int i = 0; i<ln2.size(); i++){
		if (ln2[i] > 513)
			printf(" ln2: %d ", ln2[i]);
	}
	for (int i = 0; i<lz.size(); i++){
		if (lz[i] > 513)
			printf(" lz: %d ", lz[i]);
	}
	*/
	printf("finished\n");
=======
	}
	

	printf("finished\n");
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
	
<<<<<<< HEAD
	unsigned char zeroLevelSet[HEIGHT*WIDTH] = {0};
	unsigned char data1[HEIGHT*WIDTH];
	
	vector<Pixel>::iterator it;
	for(it = lz.begin(); it<lz.end(); it++){
		zeroLevelSet[it->x*HEIGHT+it->y] = 255;
	}
	/*
	for (int i =0; i<HEIGHT; i++){
		for (int j = 0; j<WIDTH; j++){
			//printf("for 1 strta\n");
			
			data1[i*HEIGHT+j] = (char)phi[i][j]; //må kanskje forandres hvis bildet er opp ned
			//printf("%d  %f \n", data1[i*HEIGHT+j], phi[i][j]);
			//printf("for 1 done\n");
||||||| merged common ancestors
	//unsigned char data1[64*64];
	//printf("size %d", data1);
	//printf("assigned");
	//for (int i =0; i<64; i++){
		//for (int j = 0; j<64; j++){
			//printf("for 1 strta\n");
			//data1[i*64+j] = (char)image[i][j]; //må kanskje forandres hvis bildet er opp ned
			//printf("for 1 done\n");
		//}
//	}
	//printf("WRITE done");
	/*for (int i = 1; i<=HEIGHT; i++){
		for (int j = 1; j<=WIDTH; j++){
			printf(" %f ", phi[i][j]);
=======
	unsigned char data1[512*512];
	//printf("size %d", data1);
	//printf("assigned");
	for (int i =0; i<512; i++){
		for (int j = 0; j<512; j++){
			data1[i*512+j] = (char)image[i][j]; //må kanskje forandres hvis bildet er opp ned
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
		}
<<<<<<< HEAD
	}*/
	
	write_bmp(zeroLevelSet, 512, 512);
||||||| merged common ancestors
		printf("\n");
	}*/
	//write_bmp(phi, 512, 512);
	
=======
	}

	write_bmp(data1, 512, 512);
>>>>>>> 9b7eba49e98b682b563e4791a8f80de09ecbe371
	
	system("pause");

}
