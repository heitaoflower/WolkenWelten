/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "worldgen.h"

#include "../main.h"
#include "../misc/options.h"
#include "../voxel/chunk.h"
#include "../voxel/chungus.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/common.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/misc/profiling.h"

#include "island.h"
#include "labyrinth.h"
#include "landmass.h"

#include <stdio.h>
#include <stdlib.h>

worldgen worldgenList[4];
int worldgenCount = 0;
worldgen *worldgenFirstFree = NULL;

worldgen *worldgenNew(chungus *nclay){
	worldgen *wgen = NULL;
	if(worldgenFirstFree != NULL){
		wgen = worldgenFirstFree;
		worldgenFirstFree = wgen->nextFree;
	}
	if(wgen == NULL){
		wgen = &worldgenList[worldgenCount++];
	}
	wgen->clay = nclay;
	wgen->iterChance = 64;
	wgen->gx = nclay->x<<8;
	wgen->gy = nclay->y<<8;
	wgen->gz = nclay->z<<8;
	wgen->layer = (wgen->gy / CHUNGUS_SIZE);
	wgen->minX = wgen->minY = wgen->minZ = CHUNGUS_SIZE;
	int gx = wgen->gx >> 8;
	int gy = wgen->gy;
	int gz = wgen->gz >> 8;

	wgen->heightModifier          = world.heightModifier         [gx][gz];
	wgen->vegetationConcentration = world.vegetationConcentration[gx][gz] >> 5;
	wgen->islandSizeModifier      = world.islandSizeModifier     [gx][gz];
	wgen->islandCountModifier     = world.islandCountModifier    [gx][gz];
	wgen->geoworld                = world.geoworldMap            [gx][gz] + ((gy & 0xF0) << 2);
	wgen->geoIslandChance         = wgen->geoIslands = false;

	return wgen;
}

void worldgenFree(worldgen *wgen){
	wgen->nextFree = worldgenFirstFree;
	worldgenFirstFree = wgen;
}

void worldgenObelisk(worldgen *wgen, int x,int y,int z){
	int size = rngValMM(8,16);
	for(int cy=-4;cy<size;cy++){
		chungusSetB(wgen->clay,x,y+cy,z,12);
	}
}

void worldgenMonolith(worldgen *wgen, int x,int y,int z){
	chungus *clay = wgen->clay;
	for(int ox=-3;ox<6;ox++){
		for(int oy=0;oy<12;oy++){
			for(int oz=-3;oz<4;oz++){
				chungusSetB(clay,x+ox,y+oy,z+oz,0);
			}
		}
	}
	for(int ox=-1;ox<4;ox++){
		for(int oy=0;oy<10;oy++){
			chungusSetB(clay,x+ox,y+oy,z,9);
		}
	}
	for(int ox=-3;ox<6;ox++){
		for(int oy=-5;oy<0;oy++){
			for(int oz=-3;oz<4;oz++){
				chungusSetB(clay,x+ox,y+oy,z+oz,3);
			}
		}
	}
}

void worldgenFindSpawn(worldgen *wgen, int x,int z,int tries){
	if(tries > 8){
		wgen->clay->spawn = ivecNOne();
		return;
	}
	int y = 0;
	if(chungusGetHighestP(wgen->clay,x,&y,z)){
		wgen->clay->spawn = ivecNew(x,y,z);
	}else{
		worldgenFindSpawn(wgen,rngValMM(0,CHUNGUS_SIZE),rngValMM(0,CHUNGUS_SIZE),tries+1);
	}
}

void worldgenTestpattern(worldgen *wgen){
	u8 b=1;
	for(int x=0;x<CHUNK_SIZE;x+=2){
		for(int y=0;y<CHUNK_SIZE;y+=2){
			for(int z=0;z<CHUNK_SIZE;z+=2){
				chungusFill(wgen->clay,x*CHUNK_SIZE,y*CHUNK_SIZE,z*CHUNK_SIZE,b);
				if(++b > 21){b=1;}
			}
		}
	}
}

void worldgenGenerate(worldgen *wgen){
	PROFILE_START();

	int oldSeed = getRNGSeed();
	uint seed = (optionWorldSeed | (optionWorldSeed << 16));
	seed ^= (wgen->gx&0xFF00) | (wgen->gx >> 8) | ((wgen->gx&0xFF00)<<8) | ((wgen->gx&0xFF00)<<16);
	seed ^= (wgen->gy&0xFF00) | (wgen->gy >> 8) | ((wgen->gy&0xFF00)<<8) | ((wgen->gy&0xFF00)<<16);
	seed ^= (wgen->gz&0xFF00) | (wgen->gz >> 8) | ((wgen->gz&0xFF00)<<8) | ((wgen->gz&0xFF00)<<16);

	seedRNG(seed);
	if(wgen->geoworld > 188){
		wgen->geoIslandChance = false;
		wgen->geoIslands      = true;
	}else if(wgen->geoworld > 156){
		wgen->geoIslandChance = true;
		wgen->geoIslands      = false;
	}else{
		wgen->geoIslandChance = false;
		wgen->geoIslands      = false;
	}
	switch(wgen->layer){
	case 0:
	case 127:
		break;
	default:
		worldgenTestpattern(wgen);
		break;
	case 19:
	case 18:
		worldgenCluster(wgen,CHUNGUS_SIZE/52,CHUNGUS_SIZE/52,3,9);
		break;
	case 17:
		worldgenLandmass(wgen,wgen->layer - 13);
		break;
	case 16:
	case 15:
	case 14:
	case 13:
	case 12:
	case 11:
	case 10:
	case 9:
		worldgenLabyrinth(wgen);
		break;
	case 8:
		worldgenCluster(wgen,CHUNGUS_SIZE/44,CHUNGUS_SIZE/44,4,12);
		break;
	case 7:
		worldgenCluster(wgen,CHUNGUS_SIZE/38,CHUNGUS_SIZE/38,7,22);
		break;
	case 6:
		worldgenCluster(wgen,CHUNGUS_SIZE/32,CHUNGUS_SIZE/32,6,22);
		break;
	case 5:
		worldgenCluster(wgen,CHUNGUS_SIZE/22,CHUNGUS_SIZE/28,4,16);
		break;
	case 4:
		worldgenCluster(wgen,CHUNGUS_SIZE/12,CHUNGUS_SIZE/22,4,8);
		break;
	case 3:
		worldgenCluster(wgen,CHUNGUS_SIZE/8,CHUNGUS_SIZE/32,1,3);
		break;
	case 2:
		worldgenCluster(wgen,CHUNGUS_SIZE/14,CHUNGUS_SIZE/24,2,5);
		break;
	case 1:
		worldgenCluster(wgen,CHUNGUS_SIZE/18,CHUNGUS_SIZE/22,5,12);
		break;
	}
	seedRNG(oldSeed);

	PROFILE_STOP();
}
