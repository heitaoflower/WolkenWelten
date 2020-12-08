#include "character.h"
#include "../game/item.h"
#include "../game/hook.h"
#include "../mods/api_v1.h"
#include "../mods/mods.h"
#include "../network/messages.h"

#include <stdio.h>
#include <stddef.h>
#include <math.h>

character  characterList[64];
uint       characterCount = 0;

character *characterNew(){
	character *c = NULL;

	for(uint i=0;i<characterCount;i++){
		if(characterList[i].eMesh == NULL){
			c = &characterList[i];
			break;
		}
	}
	if(c == NULL){
		if(characterCount >= countof(characterList)){
			fprintf(stderr,"characterList Overflow!\n");
			return NULL;
		}
		c = &characterList[characterCount++];
	}
	characterInit(c);

	return c;
}

void characterFree(character *c){
	c->eMesh = NULL;
	c->hp = -1;
	if(c->hook != NULL){
		hookFree(c->hook);
		c->hook = NULL;
	}
}

int characterGetHP(const character *c){
	return c->hp;
}

int characterGetMaxHP(const character *c){
	return c->maxhp;
}

void characterToggleGlider(character *c){
	if(c == NULL){return;}
	if((itemIsEmpty(&c->equipment[CHAR_EQ_GLIDER])) || (c->equipment[CHAR_EQ_GLIDER].ID != I_Glider)){return;}
	c->flags ^= CHAR_GLIDE;
}

void characterAddCooldown(character *c, int cooldown){
	c->actionTimeout = -cooldown;
}

void characterSetPos(character *c, const vec pos){
	c->pos = pos;
}

void characterSetRot(character *c, const vec rot){
	c->rot = rot;
}

void characterSetVelocity(character *c, const vec vel){
	c->vel = vel;
}

void characterAddInaccuracy(character *c, float inc){
	c->inaccuracy += inc;
}

int characterGetItemAmount(const character *c, u16 itemID){
	int amount = 0;
	for(unsigned int i=0;i<40;i++){
		if(c->inventory[i].ID == itemID){
			amount += c->inventory[i].amount;
		}
	}
	return amount;
}

int characterDecItemAmount(character *c, u16 itemID,int amount){
	int ret=0;

	if(amount == 0){return 0;}
	for(uint i=0;i<40;i++){
		if(c->inventory[i].ID == itemID){
			if(c->inventory[i].amount > amount){
				itemDecStack(&c->inventory[i],amount);
				return amount;
			}else{
				amount -= c->inventory[i].amount;
				ret    += c->inventory[i].amount;
				c->inventory[i] = itemEmpty();
			}
		}
	}
	return ret;
}

int characterPickupItem(character *c, u16 itemID,int amount){
	int a = 0;
	item ci = itemNew(itemID,amount);
	if(getStackSizeDispatch(&ci) == 1){
		ci.amount = amount;
		for(uint i=0;i<40;i++){
			if(itemIsEmpty(&c->inventory[i])){
				c->inventory[i] = ci;
				sfxPlay(sfxPock,.8f);
				return 0;
			}
		}
		return -1;
	}

	for(uint i=0;i<40;i++){
		if(a >= amount){break;}
		if(itemCanStack(&c->inventory[i],itemID)){
			a += itemIncStack(&c->inventory[i],amount - a);
		}
	}
	for(uint i=0;i<40;i++){
		if(a >= amount){break;}
		if(itemIsEmpty(&c->inventory[i])){
			c->inventory[i] = itemNew(itemID,amount - a);
			a += c->inventory[i].amount;
		}
	}

	if(a == amount){
		sfxPlay(sfxPock,.8f);
		return 0;
	}else if(a != 0){
		sfxPlay(sfxPock,.8f);
	}else if(a == 0){
		return -1;
	}
	return a;
}

bool characterPlaceBlock(character *c,item *i){
	if(c->actionTimeout < 0)              { return false; }
	ivec los = characterLOSBlock(c,true);
	if(los.x < 0)                         { return false; }
	if((characterCollision(c->pos)&0xFF0)){ return false; }
	if(!itemDecStack(i,1))                { return false; }

	worldSetB(los.x,los.y,los.z,i->ID);
	if((characterCollision(c->pos)&0xFF0) != 0){
		worldSetB(los.x,los.y,los.z,0);
		itemIncStack(i,1);
		return false;
	} else {
		msgPlaceBlock(los.x,los.y,los.z,i->ID);
		sfxPlay(sfxPock,1.f);
		characterStartAnimation(c,0,240);
		characterAddCooldown(c,50);
		return true;
	}
}

bool characterHP(character *c, int addhp){
	c->hp += addhp;
	if(c->hp <= 0){
		characterDie(c);
		return true;
	}
	if(c->hp > c->maxhp){
		c->hp = c->maxhp;
	}
	return false;
}

bool characterDamage(character *c, int hp){
	return characterHP(c,-hp);
}

void characterEmptyInventory(character *c){
	for(uint i=0;i<40;i++){
		c->inventory[i] = itemEmpty();
	}
}

item *characterGetItemBarSlot(character *c, uint i){
	if(i >= 40){return NULL;}
	return &c->inventory[i];
}

item *characterGetActiveItem(character *c){
	return characterGetItemBarSlot(c,c->activeItem);
}

void characterSetItemBarSlot(character *c, uint i, item *itm){
	if(i >= 40){return;}
	c->inventory[i] = *itm;
}

void characterSetActiveItem(character *c,  int i){
	if(i > 9){i = 0;}
	if(i < 0){i = 9;}
	if((uint)i != c->activeItem){
		characterStartAnimation(c,5,200);
	}
	c->activeItem = i;
}

void characterSwapItemSlots(character *c, uint a,uint b){
	if(a >= 40){return;}
	if(b >= 40){return;}

	item tmp = c->inventory[a];
	c->inventory[a] = c->inventory[b];
	c->inventory[b] = tmp;
}

ivec characterLOSBlock(const character *c, int returnBeforeBlock) {
	const vec cv = vecMulS(vecDegToVec(c->rot),0.0625f);
	vec       cp = vecAdd(c->pos,vecNew(0,0.5,0));
	ivec      l  = ivecNewV(cp);

	for(int i=0;i<48;i++){
		cp = vecAdd(cp,cv);
		const ivec ip = ivecNewV(cp);
		if(!ivecEq(ip,l)){
			if(worldGetB(ip.x,ip.y,ip.z) > 0){
				if(returnBeforeBlock){
					return l;
				}
				return ip;
			}
			l = ip;
		}
	}
	return ivecNOne();
}

u32 characterCollision(const vec c){
	u32 col = 0;
	const float wd = 0.28f;
	const float WD = wd*1.75f;

	if(checkCollision(c.x-wd,c.y+0.5f,c.z   )){col |=  0x10;}
	if(checkCollision(c.x+wd,c.y+0.5f,c.z   )){col |=  0x20;}
	if(checkCollision(c.x   ,c.y+0.5f,c.z-wd)){col |=  0x40;}
	if(checkCollision(c.x   ,c.y+0.5f,c.z+wd)){col |=  0x80;}

	if(checkCollision(c.x-wd,c.y-1.f ,c.z   )){col |=   0x1;}
	if(checkCollision(c.x+wd,c.y-1.f ,c.z   )){col |=   0x2;}
	if(checkCollision(c.x   ,c.y-1.f ,c.z-wd)){col |=   0x4;}
	if(checkCollision(c.x   ,c.y-1.f ,c.z+wd)){col |=   0x8;}

	if(checkCollision(c.x-WD,c.y-0.7f,c.z   )){col |= 0x100;}
	if(checkCollision(c.x+WD,c.y-0.7f,c.z   )){col |= 0x200;}
	if(checkCollision(c.x   ,c.y-0.7f,c.z-WD)){col |= 0x400;}
	if(checkCollision(c.x   ,c.y-0.7f,c.z+WD)){col |= 0x800;}

	return col;
}

void characterMove(character *c, const vec mov){
	const float yaw   = c->rot.yaw;

	if(c->flags & CHAR_NOCLIP){
		float s = 0.2f;
		if(c->flags & CHAR_SNEAK){ s = 2.5f;}
		c->gvel    = vecMulS(vecDegToVec(c->rot),mov.z*(-s));
		c->gvel.x += cos((yaw)*PI/180)*mov.x*s;
		c->gvel.z += sin((yaw)*PI/180)*mov.x*s;
		c->gvel.y += mov.y;
	}else{
		const float s = 0.05f;
		c->gvel.y = mov.y;
		c->gvel.x = (cos((yaw+90)*PI/180)*mov.z*s);
		c->gvel.z = (sin((yaw+90)*PI/180)*mov.z*s);

		c->gvel.x += cos((yaw)*PI/180)*mov.x*s;
		c->gvel.z += sin((yaw)*PI/180)*mov.x*s;
	}
}

void characterRotate(character *c, const vec rot){
	c->rot = vecAdd(c->rot,rot);

	if(c->rot.pitch < -90.f){
		 c->rot.pitch = -90.f;
	}else if(c->rot.pitch >  90.f){
		 c->rot.pitch =  90.f;
	}
	if(c->rot.yaw < 0.f){
		 c->rot.yaw += 360.f;
	}else if(c->rot.yaw >  360.f){
		 c->rot.yaw -= 360.f;
	}
}

void characterStartAnimation(character *c, int index, int duration){
	c->animationIndex = index;
	c->animationTicksLeft = c->animationTicksMax = duration;
}

bool characterItemReload(character *c, item *i, int cooldown){
	const int MAGSIZE = getMagSizeDispatch(i);
	const int AMMO    = getAmmunitionDispatch(i);
	int ammoleft      = characterGetItemAmount(c,AMMO);

	if(c->actionTimeout < 0)      {return false;}
	if(itemGetAmmo(i) == MAGSIZE) {return false;}
	if(ammoleft <= 0)             {return false;}

	ammoleft = MIN(MAGSIZE,ammoleft);
	characterDecItemAmount(c, AMMO, itemIncAmmo(i,ammoleft));

	characterAddCooldown(c,cooldown);
	sfxPlay(sfxHookReturned,1.f);
	characterStartAnimation(c,2,MAX(cooldown*2,500));

	return true;
}

bool characterTryToShoot(character *c, item *i, int cooldown, int bulletcount){
	if(c->actionTimeout < 0){return false;}
	if(itemGetAmmo(i) < bulletcount){
		if(characterItemReload(c,i,256)){return false;}
		sfxPlay(sfxHookFire,0.3f);
		characterAddCooldown(c,64);
		characterStartAnimation(c,3,250);
		return false;
	}
	itemDecAmmo(i,bulletcount);
	characterAddCooldown(c,cooldown);
	characterStartAnimation(c,1,250);
	return true;
}

bool characterTryToUse(character *c, item *i, int cooldown, int itemcount){
	if(c->actionTimeout < 0){return false;}
	if(i->amount < itemcount){
		sfxPlay(sfxHookFire,0.3f);
		characterAddCooldown(c,64);
		return false;
	}
	itemDecStack(i,itemcount);
	characterAddCooldown(c,cooldown);
	return true;
}

void characterSetInventoryP(character *c, const packet *p){
	if(c == NULL){return;}
	uint max = MIN(40,packetLen(p)/4);
	uint ii = 0;
	for(uint i=0;i<max;i++){
		c->inventory[i].ID     = p->v.u16[ii++];
		c->inventory[i].amount = p->v.i16[ii++];
	}
}

void characterSetEquipmentP(character *c, const packet *p){
	if(c == NULL){return;}
	uint max = MIN(3,packetLen(p)/4);
	uint ii = 0;
	for(uint i=0;i<max;i++){
		c->equipment[i].ID     = p->v.u16[ii++];
		c->equipment[i].amount = p->v.i16[ii++];
	}
}

float characterGetMaxHookLen(const character *c){
	if(!itemIsEmpty(&c->equipment[CHAR_EQ_HOOK])){
		if(c->equipment[CHAR_EQ_HOOK].ID == I_Hook){return 256.f;}
	}
	return 64;
}

float characterGetHookWinchS(const character *c){
	if(!itemIsEmpty(&c->equipment[CHAR_EQ_HOOK])){
		if(c->equipment[CHAR_EQ_HOOK].ID == I_Hook){return 0.1f;}
	}
	return 0.04f;
}

character *characterGetByBeing(being b){
	const uint i = beingID(b);
	if(beingType(b) != BEING_CHARACTER){ return NULL; }
	if(i >= characterCount)            { return NULL; }
	return &characterList[i];
}

uint characterGetBeing(const character *c){
	if(c == NULL){return 0;}
	return beingCharacter(c - &characterList[0]);
}

character *characterClosest(const vec pos, float maxDistance){
	for(uint i=0;i<characterCount;i++){
		if(characterList[i].hp <= 0){continue;}
		const float d = vecMag(vecSub(pos,characterList[i].pos));
		if(d > maxDistance){continue;}
		return &characterList[i];
	}
	return NULL;
}

void characterAddRecoil(character *c, float recoil){
	const vec vel = vecMulS(vecDegToVec(c->rot),.01f);
	c->vel = vecAdd(c->vel, vecMulS(vel,-0.75f*recoil));
	c->rot = vecAdd(c->rot, vecNew((rngValf()-0.5f) * recoil, (rngValf()-.8f) * recoil, 0.f));
}
