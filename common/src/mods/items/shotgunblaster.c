static const int ITEMID=264;

#include "../api_v1.h"

void shotgunblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,36, 13,18); // Crystal(36) + Hematite Ore(18) -> Shotgunblaster(1)
}

mesh *shotgunblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

int shotgunblasterGetStackSize(item *cItem){
	(void)cItem;

	return 1;
}

bool shotgunblasterPrimaryAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) < 32){return false;}
	characterDecItemAmount(cChar, 265, 32);
	characterAddCooldown(cChar,256);
	beamblast(cChar,1.f,1.f,0.04f,6,48,32.f,1.f);
	return true;
}

bool shotgunblasterSecondaryAction(item *cItem,character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	if(characterGetItemAmount(cChar,265) < 32){return false;}
	characterDecItemAmount(cChar, 265, 32);
	characterAddCooldown(cChar,512);
	beamblast(cChar,1.f,1.f,0.02f,6,128,32.f,4.f);
	return true;
}

float shotgunblasterGetInaccuracy(item *cItem){
	(void)cItem;

	return 24.f;
}

int shotgunblasterGetAmmunition(item *cItem){
	(void)cItem;
	
	return 265;
}