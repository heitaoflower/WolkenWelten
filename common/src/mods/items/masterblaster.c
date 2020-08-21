static const int ITEMID=262;
static const int MAGSIZE=90;

#include "../api_v1.h"

void masterblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,24, 13,12); // Crystal(24) + Hematite Ore(12) -> Masterblaster(1)
}

mesh *masterblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int masterblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool masterblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)to;

	if(!characterTryToShoot(cChar,cItem,350,45)){return false;}
	beamblast(cChar,4.f,8.f,2.f,1024,1,32.f,1.f);
	return true;
}

bool masterblasterSecondaryAction(item *cItem, character *cChar, int to){
	(void)cItem;
	(void)to;

	if(!characterTryToShoot(cChar,cItem,50,5)){return false;}
	beamblast(cChar,3.f,0.1f,2.f,1,1,16.f,1.f);
	return true;
}

bool masterblasterTertiaryAction(item *cItem, character *cChar, int to){
	(void)to;
	return characterItemReload(cChar, cItem, 200);
}

int masterblasterGetAmmunition(item *cItem){
	(void)cItem;

	return 265;
}

int masterblasterGetMagSize(item *cItem){
	(void)cItem;

	return MAGSIZE;
}
