#include "bunny.h"

#include "../game/animal.h"
#include "../game/entity.h"
#include "../game/itemDrop.h"
#include "../network/server.h"
#include "../voxel/bigchungus.h"
#include "../../../common/src/game/item.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"

static int animalCheckHeat(animal *e){
	if(e->pregnancy > 0){return 0;}
	if((e->age > 20) && (rngValM( 128) == 0)){
		animal *cAnim;
		if(animalClosestAnimal(e,&cAnim,e->type,0,0) < 192.f){
			if(cAnim->age > 20){
				e->state = ANIMAL_S_HEAT;
				return 1;
			}
		}
	}
	return 0;
}

static void animalAgeing(animal *e){
	if(e->age < 125){
		if(rngValM(1<<8) == 0){e->age++;}
	}
	if(e->age > 64){
		if(rngValM(1<<12) <= (uint)(e->age-64)){e->health = 0;}
	}
}

static void animalSleepyness(animal *e){
	if (rngValM( 32) == 0){e->sleepy--;}
	if((e->state != ANIMAL_S_FLEE) && (e->state != ANIMAL_S_FIGHT) && (e->sleepy < 16)){
		e->state = ANIMAL_S_SLEEP;
		return;
	}
	if(e->sleepy <  8){
		e->state = ANIMAL_S_SLEEP;
		return;
	}
}

static void animalHunger(animal *e){
	if(rngValM(32) == 0){
		e->hunger--;
	}
	if(e->state == ANIMAL_S_FOOD_SEARCH){return;}
	if(e->state == ANIMAL_S_EAT){return;}
	if(e->hunger < (int)rngValM(32)){
		e->state = ANIMAL_S_FOOD_SEARCH;
		return;
	}
}

static void animalSLoiter(animal *e){
	if(e->hunger < 64){
		const u8 cb = worldGetB(e->pos.x,e->pos.y-.6f,e->pos.z);
		if((cb == 2) && (rngValM(128) == 0)){
			worldSetB(e->pos.x,e->pos.y-.6f,e->pos.z,1);
			e->hunger += 48;
		}
	}
	if(animalCheckHeat(e)){return;}

	if(rngValM( 8) == 0){
		e->grot.yaw = e->rot.yaw + ((rngValf()*2.f)-1.f)*4.f;
	}
	if(rngValM(16) == 0){
		e->gvel = vecZero();
	}
	if(rngValM(48) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*10.f;
	}
	if(rngValM(64) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM(32) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}

	if(rngValM(256) < (uint)(127-e->age)){
		e->state = ANIMAL_S_PLAYING;
		return;
	}
}

static void animalSSleep(animal *e){
	e->gvel = vecZero();

	if(e->sleepy > 120){
		e->state      = ANIMAL_S_LOITER;
		e->grot.pitch = 0.f;
		return;
	}else if(e->sleepy > 64){
		if(rngValM(1<<9) <= (uint)(e->sleepy-64)*2){
			e->state      = ANIMAL_S_LOITER;
			e->grot.pitch = 0.f;
			return;
		}
	}

	if(e->flags & ANIMAL_BELLYSLEEP){
		e->grot.pitch = -90.f;
	}else{
		e->grot.pitch =  90.f;
	}
	if(rngValM(4) == 0){
		e->sleepy++;
	}
	if(rngValM(48) == 0){
		if(e->health < animalGetMaxHealth(e)){e->health++;}
	}
}

static void animalSHeat(animal *e){
	animal *cAnim;
	float dist;
	if(e->pregnancy > 0){
		e->state = ANIMAL_S_LOITER;
	}
	if(e->flags & ANIMAL_MALE){
		dist = animalClosestAnimal(e,&cAnim,e->type,ANIMAL_MALE,0);
	}else{
		dist = animalClosestAnimal(e,&cAnim,e->type,ANIMAL_MALE,ANIMAL_MALE);
	}
	if((dist > 256.f) || (cAnim == NULL)){ e->state = 0; }

	if((e->hunger < 12) || (e->sleepy < 12)){
		e->state = ANIMAL_S_LOITER;
		return;
	}

	if(dist < 2.f){
		e->state     =  ANIMAL_S_LOITER;
		cAnim->state = ANIMAL_S_LOITER;
		if(e->flags & ANIMAL_MALE){
			e->pregnancy = 64;
		}else if(e->flags & ANIMAL_MALE){
			e->pregnancy = 64;
		}
		return;
	}
	if(cAnim == NULL){return;}
	const vec caNorm = vecNorm(vecSub(e->pos,cAnim->pos));
	const vec caVel  = vecMulS(caNorm,-0.03f);
	const vec caRot  = vecVecToDeg(caNorm);

	e->gvel.x  = caVel.x;
	e->gvel.z  = caVel.z;

	e->rot.yaw = -caRot.yaw + 180.f;
}

static void animalSFight(animal *e){
	if( rngValM(32) == 0){e->hunger--;}
	if( rngValM(24) == 0){e->sleepy--;}
	if(e->type != 2){
		if((rngValM(20) == 0) && !(e->flags & ANIMAL_FALLING)){
			e->vel.y = 0.03f;
		}
	}
	if(rngValM(24) == 0){
		animal *cAnim;
		float dist = animalClosestAnimal(e,&cAnim,e->type,0,0);
		if((dist < 32.f) && (cAnim != NULL)){
			e->state   =  ANIMAL_S_LOITER;
			e->flags  &= ~ANIMAL_AGGRESIVE;
		}
	}
}


static void animalSFlee(animal *e){
	if(rngValM(32) == 0){e->hunger--;}
	if(rngValM(24) == 0){e->sleepy--;}
	if(rngValM(16) == 0){
		animal *cAnim;
		float dist = animalClosestAnimal(e,&cAnim,e->type,0,0);
		if((dist < 32.f) && (cAnim != NULL) && (cAnim->state == ANIMAL_S_FIGHT)){
			e->state   =  ANIMAL_S_LOITER;
			e->flags  &= ~ANIMAL_AGGRESIVE;
		}
	}
}

static void animalFightOrFlight(animal *e){
	character *cChar;
	float dist = animalClosestPlayer(e,&cChar);

	if(e->state == ANIMAL_S_FIGHT){
		if((cChar == NULL) || (dist > 16.f)){
			e->state   =  ANIMAL_S_LOITER;
			e->flags  &= ~ANIMAL_AGGRESIVE;
			addPriorityAnimal(e-animalList);
			return;
		}else{
			vec caNorm = vecNorm(vecNew(cChar->pos.x - e->pos.x,0.f, cChar->pos.z - e->pos.z));
			vec caVel  = vecMulS(caNorm,0.03f);
			vec caRot  = vecVecToDeg(caNorm);

			e->gvel.x  = caVel.x;
			e->gvel.z  = caVel.z;
			e->rot.yaw = -caRot.yaw;

			if((dist < 1.5f)){
				e->gvel.x = 0;
				e->gvel.z = 0;
				if(rngValM(6)==0){
					int target = getClientByCharacter(cChar);
					int dmg = 1;
					if(target < 0){return;}
					if(rngValM(8)==0){dmg = 4;}
					msgBeingDamage(target,dmg,2,beingCharacter(target),-1,e->pos);
				}
			}
			addPriorityAnimal(e-animalList);
		}
	}else if(e->state == ANIMAL_S_FLEE){
		if((cChar == NULL) || (dist > 32.f)){
			e->state = ANIMAL_S_LOITER;
			addPriorityAnimal(e-animalList);
			return;
		}else{
			vec caNorm = vecNorm(vecNew(e->pos.x - cChar->pos.x,0.f, e->pos.z - cChar->pos.z));
			vec caVel  = vecMulS(caNorm,0.03f);
			vec caRot  = vecVecToDeg(caNorm);

			e->gvel.x  = caVel.x;
			e->gvel.z  = caVel.z;
			e->rot.yaw = -caRot.yaw;
			if((dist < 3.f) && (rngValM(8)==0)){
				e->state = ANIMAL_S_FIGHT;
				e->flags |= ANIMAL_AGGRESIVE;
			}
			addPriorityAnimal(e-animalList);
		}
	}else{
		float fd = 9.f;
		if(e->state == ANIMAL_S_SLEEP){fd = 3.f;}
		if(e->state == ANIMAL_S_PLAYING){fd = 15.f;}
		if((cChar != NULL) && (dist < fd)){
			if((e->state == ANIMAL_S_SLEEP) && !(e->flags & ANIMAL_FALLING)){
				e->vel.y   = 0.04f;
			}
			e->grot.pitch = 0.f;
			if(e->state == ANIMAL_S_SLEEP){
				e->state = ANIMAL_S_FIGHT;
				e->flags |= ANIMAL_AGGRESIVE;
			}else{
				e->state = ANIMAL_S_FLEE;
			}
			addPriorityAnimal(e-animalList);
			return;
		}
	}
}

static void animalSPlayful(animal *e){
	if (rngValM( 8) == 0){e->sleepy--;}
	if(animalCheckHeat(e)){return;}

	if((rngValM( 8) == 0) && !(e->flags & ANIMAL_FALLING)){
		e->vel.y = 0.03f;
	}
	if (rngValM(12) == 0){
		e->gvel = vecZero();
	}
	if (rngValM(16) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if (rngValM(16) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if (rngValM(24) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(-e->rot.yaw,0.f,0.f)),0.02f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}

	if (rngValM(1024) < (uint)(e->age*6)){
		e->state = ANIMAL_S_LOITER;
		return;
	}
}

static void animalSFoodSearch(animal *e){
	const u8 cb = worldGetB(e->pos.x,e->pos.y-1,e->pos.z);
	if(cb == 2){
		e->gvel = vecZero();
		e->state = ANIMAL_S_EAT;
		return;
	}
	if(e->hunger > 48){
		if((int)rngValM(48) < e->hunger-48){
			e->state = ANIMAL_S_LOITER;
		}
	}

	if(rngValM(4) == 0){
		e->grot.pitch = ((rngValf()*2.f)-1.f)*16.f;
	}
	if(rngValM(4) == 0){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
	}
	if(rngValM(6) == 0){
		vec dir = vecMulS(vecDegToVec(vecNew(-e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
	}
	if(rngValM(16) == 0){
		e->gvel = vecZero();
	}
}

static void animalSEat(animal *e){
	const u8 cb = worldGetB(e->pos.x,(int)e->pos.y-1,e->pos.z);
	if(cb != 2){
		e->state = ANIMAL_S_FOOD_SEARCH;
		return;
	}
	if(e->hunger > 48){
		if((int)rngValM(48) < e->hunger-48){
			e->state = ANIMAL_S_LOITER;
		}
	}
	if(rngValM( 8) == 0){
		e->hunger++;
	}
	if(rngValM(32) == 0){
		if(e->health < animalGetMaxHealth(e)){e->health++;}
	}
	if(rngValM(64) == 0){
		worldSetB(e->pos.x,(int)e->pos.y-1,e->pos.z,1);
	}
}

static void animalDoPoop(animal *e){
	item ipoop = itemNew(I_Poop,1);
	itemDropNewP(e->pos, &ipoop);
}

static void animalPregnancy(animal *e){
	if(e->flags & ANIMAL_MALE){e->pregnancy = -1; return;}
	if(e->pregnancy  <  0)    {return;}
	if(e->age        < 21)    {e->pregnancy = -1;}

	if(rngValM(8) == 0){--e->pregnancy;}
	if(e->pregnancy == 0){
		animal *cAnim = animalNew(vecNew(e->pos.x+((rngValf()*2.f)-1.f),e->pos.y+.4f,e->pos.z+((rngValf()*2.f)-1.f)),e->type);
		if(cAnim != NULL){
			cAnim->age = 1;
		}
		e->pregnancy = -1;
		animalDoPoop(e);
	}
}

static void animalPoop(animal *e){
	if(e->hunger < 24){return;}

	if(e->hunger < 48){
		if(rngValM(128) == 0){animalDoPoop(e);}
	}else{
		if(rngValM( 78) == 0){animalDoPoop(e);}
	}
}

#include <stdio.h>
static void animalSocialDistancing(animal *e){
	animal *cAnim;
	float dist = animalClosestAnimal(e,&cAnim,e->type,0,0);
	if(dist < 1.75f){
		e->grot.yaw = ((rngValf()*2.f)-1.f)*360.f;
		vec dir = vecMulS(vecDegToVec(vecNew(-e->rot.yaw,0.f,0.f)),0.01f);
		e->gvel.x = dir.x;
		e->gvel.z = dir.z;
		if(!(e->flags & ANIMAL_FALLING)){
			e->vel.y  = 0.03f;
		}
	}
}

void animalThinkBunny(animal *e){
	animalCheckSuffocation(e);
	animalFightOrFlight(e);
	animalAgeing(e);
	animalSleepyness(e);
	animalHunger(e);
	animalPregnancy(e);
	animalPoop(e);
	animalSocialDistancing(e);

	switch(e->state){
	default:
	case ANIMAL_S_LOITER:
		animalSLoiter(e);
		break;
	case ANIMAL_S_FLEE:
		animalSFlee(e);
		break;
	case ANIMAL_S_HEAT:
		animalSHeat(e);
		break;
	case ANIMAL_S_SLEEP:
		animalSSleep(e);
		break;
	case ANIMAL_S_PLAYING:
		animalSPlayful(e);
		break;
	case ANIMAL_S_FOOD_SEARCH:
		animalSFoodSearch(e);
		break;
	case ANIMAL_S_EAT:
		animalSEat(e);
		break;
	case ANIMAL_S_FIGHT:
		animalSFight(e);
		break;
	}
}