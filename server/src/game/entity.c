#include "entity.h"

#include "../main.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>

entity entityList[1<<14];
int entityCount = 0;
entity *entityFirstFree = NULL;

entity *entityNew(const vec pos, const vec rot){
	entity *e = NULL;
	if(entityFirstFree == NULL){
		e = &entityList[entityCount++];
	}else{
		e = entityFirstFree;
		entityFirstFree = e->nextFree;
		if(entityFirstFree == e){
			entityFirstFree = NULL;
		}
	}
	entityReset(e);
	e->pos        = pos;
	e->rot        = rot;
	return e;
}

void entityFree(entity *e){
	if(e == NULL){return;}
	e->nextFree = entityFirstFree;
	entityFirstFree = e;
	if(e->nextFree == NULL){
		e->nextFree = e;
	}
}

void entityUpdateAll(){
	for(int i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL){ continue; }
		if(!(entityList[i].flags & ENTITY_UPDATED)){
			entityUpdate(&entityList[i]);
		}
		entityList[i].flags &= ~ENTITY_UPDATED;
	}
}
