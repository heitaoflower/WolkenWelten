static const int ITEMID=261;

#include "../api_v1.h"

void blasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,12, 13,6); // Crystal(12) + Hematite Ore(6) -> Blaster(1)
}

mesh *blasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool blasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool blasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool blasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	characterAddCooldown(cChar,100);
	beamblast(cChar,1.2f,1.0f,0.15f,6,1,18.f,1.f);
	return true;
}

bool blasterActivateItem(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	characterAddCooldown(cChar,20);
	beamblast(cChar,0.8f,0.5f,0.05f,6,1,18.f,1.f);
	return true;
}