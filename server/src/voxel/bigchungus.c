#include "bigchungus.h"

#include "../network/server.h"
#include "../game/entity.h"
#include "../game/blockType.h"
#include "../game/blockMining.h"
#include "../../../common/src/misc.h"
#include "../misc/noise.h"
#include "../misc/options.h"
#include "../voxel/chungus.h"
#include "../voxel/chunk.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

bigchungus world;

float chungusRoughDistance(character *cam, float x, float y,float z) {
	x = x * CHUNGUS_SIZE + CHUNGUS_SIZE/2;
	y = y * CHUNGUS_SIZE + CHUNGUS_SIZE/2;
	z = z * CHUNGUS_SIZE + CHUNGUS_SIZE/2;

	float xdiff = fabsf(x-cam->x);
	float ydiff = fabsf(y-cam->y);
	float zdiff = fabsf(z-cam->z);

	return sqrtf((xdiff*xdiff)+(ydiff*ydiff)+(zdiff*zdiff));
}

void bigchungusInit(bigchungus *c){
	memset(c->chungi,0,256*128*256*sizeof(chunk *));

	generateNoise(optionWorldSeed ^ 0x84407db3);
	memcpy(c->vegetationConcentration,heightmap,sizeof(heightmap));
	generateNoise(optionWorldSeed ^ 0xc2fb18f4);
	memcpy(c->islandSizeModifier,heightmap,sizeof(heightmap));
	generateNoise(optionWorldSeed ^ 0x1ab033cF);
	memcpy(c->islandCountModifier,heightmap,sizeof(heightmap));
	generateNoise(optionWorldSeed ^ 0xF79610E3);
	memcpy(c->geoworldMap,heightmap,sizeof(heightmap));
}

void bigchungusFree(bigchungus *c){
	for(int x=0;x<256;x++){
		for(int y=0;y<128;y++){
			for(int z=0;z<256;z++){
				chungusFree(c->chungi[x][y][z]);
			}
		}
	}
	memset(c->chungi,0,256*128*256*sizeof(chunk *));
}

chungus *bigchungusGetChungus(bigchungus *c, int x,int y,int z) {
	if((x|y|z)&(~0xFF)){return NULL;}
	chungus *chng = c->chungi[x&0xFF][y&0x7F][z&0xFF];
	if(chng == NULL){
		chng = c->chungi[x&0xFF][y&0x7F][z&0xFF] = chungusNew(x*CHUNGUS_SIZE,y*CHUNGUS_SIZE,z*CHUNGUS_SIZE);
	}
	return chng;
}

chunk *bigchungusGetChunk(bigchungus *c, int x, int y, int z){
	chungus *chng = bigchungusGetChungus(c,(x>>8)&0xFF,(y>>8)&0xFF,(z>>8)&0xFF);
	if(chng == NULL){return NULL;}
	chunk *chnk = chungusGetChunk(chng,x&0xFF,y&0xFF,z&0xFF);
	return chnk;
}

uint8_t bigchungusGetB(bigchungus *c, int x,int y,int z) {
	chungus *chng;
	chng = bigchungusGetChungus(c,x/CHUNGUS_SIZE,y/CHUNGUS_SIZE,z/CHUNGUS_SIZE);
	if(chng == NULL){ return 0; }
	return chungusGetB(chng,x%CHUNGUS_SIZE,y%CHUNGUS_SIZE,z%CHUNGUS_SIZE);
}

bool bigchungusGetHighestP(bigchungus *c, int x,int *retY, int z) {
	chungus *chng;

	if(x < 0){return 0;}
	if(z < 0){return 0;}
	int cx = x/CHUNGUS_SIZE;
	int cz = z/CHUNGUS_SIZE;
	if(cx >= 256){return 0;}
	if(cz >= 256){return 0;}
	x = x - (cx*CHUNGUS_SIZE);
	z = z - (cz*CHUNGUS_SIZE);

	for(int cy=127;cy >= 0;cy--){
		chng = c->chungi[cx][cy][cz];
		if(chng == NULL){
			chng = c->chungi[cx][cy][cz] = chungusNew(cx*CHUNGUS_SIZE,cy*CHUNGUS_SIZE,cz*CHUNGUS_SIZE);
		}
		int y;
		if(chungusGetHighestP(chng,x,&y,z)){
			*retY = y+(cy*CHUNGUS_SIZE);
			return true;
		}
	}
	return false;
}

void bigchungusSetB(bigchungus *c, int x,int y,int z,uint8_t block){
	chungus *chng;
	int cx = (x / CHUNGUS_SIZE) & 0xFF;
	int cy = (y / CHUNGUS_SIZE) & 0x7F;
	int cz = (z / CHUNGUS_SIZE) & 0xFF;
	chng = c->chungi[cx][cy][cz];
	if(chng == NULL){
		c->chungi[cx][cy][cz] = chng = chungusNew(cx*CHUNGUS_SIZE,cy*CHUNGUS_SIZE,cz*CHUNGUS_SIZE);
	}
	chungusSetB(chng,x%CHUNGUS_SIZE,y%CHUNGUS_SIZE,z%CHUNGUS_SIZE,block);
}

void bigchungusBox(bigchungus *c, int x,int y,int z, int w,int h,int d,uint8_t block){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				bigchungusSetB(c,cx+x,cy+y,cz+z,block);
			}
		}
	}
}

void bigchungusBoxMine(bigchungus *c, int x,int y,int z, int w,int h,int d){
	for(int cx=0;cx<w;cx++){
		for(int cy=0;cy<h;cy++){
			for(int cz=0;cz<d;cz++){
				uint8_t b = bigchungusGetB(c,cx+x,cy+y,cz+z);
				if(b==0){continue;}
				bigchungusSetB(c,cx+x,cy+y,cz+z,0);/*
				if(rngValM(8)==0){
					blockMiningDropItemsPos(cx+x,cy+y,cz+z,b);
				}*/
			}
		}
	}
}

void bigchungusGenSpawn(bigchungus *c){
	for(int x=127;x<=129;x++){
		for(int y=1;y<=3;y++){
			for(int z=127;z<=129;z++){
				if(c->chungi[x][y][z] == NULL){
					c->chungi[x][y][z] = chungusNew(x*CHUNGUS_SIZE,y*CHUNGUS_SIZE,z*CHUNGUS_SIZE);
				}
				if(c->chungi[x][y][z]->spawnx >= 0){
					c->spawnx = c->chungi[x][y][z]->spawnx + (CHUNGUS_SIZE * x);
					c->spawny = c->chungi[x][y][z]->spawny + (CHUNGUS_SIZE * y);
					c->spawnz = c->chungi[x][y][z]->spawnz + (CHUNGUS_SIZE * z);
				}
			}
		}
	}
}

void bigchungusGetSpawnPos(bigchungus *c, int *x, int *y, int *z){
	*x = c->spawnx;
	*y = c->spawny;
	*z = c->spawnz;
}

void bigchungusFreeFarChungi(bigchungus *c){
	int len = chungusGetActiveCount();
	for(int i=0;i<len;i++){
		chungus *chng = chungusGetActive(i);
		if(chng->nextFree != NULL){continue;}
		int x = (int)chng->x>>8;
		int y = (int)chng->y>>8;
		int z = (int)chng->z>>8;
		if((x >= 127) && (x <= 129) && (y >= 1) && (y <= 3) && (z >= 127) && (z <= 129)){continue;}

		for(int ii=0;ii<clientCount;++ii){
			if(chungusRoughDistance(clients[ii].c,x,y,z) < 768.f){ goto chungiDontFree; }
		}
		chungusFree(c->chungi[x][y][z]);
		c->chungi[x][y][z] = NULL;
		chungiDontFree:;
	}
}
