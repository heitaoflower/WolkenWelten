#pragma once
#include "../../../common/src/common.h"
#include "../../../common/src/game/animal.h"

extern animal animalList[1<<12];
extern uint animalCount;

void animalDrawAll        ();
void animalUpdateAll      ();
int  animalHitCheck       (const vec pos, float mdd, int dmg, int cause, u16 iteration);
void animalSyncFromServer (const packet *p);
void animalGotHitPacket   (const packet *p);
