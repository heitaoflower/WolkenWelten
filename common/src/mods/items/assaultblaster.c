static const int ITEMID=263;

#include "../api_v1.h"

void assaultblasterInit(){
	(void)ITEMID;
	recipeAdd2I(ITEMID,1, 18,24, 13,12); // Crystal(24) + Hematite Ore(12) -> Assaultblaster(1)
}

mesh *assaultblasterGetMesh(item *cItem){
	(void)cItem;

	return meshMasterblaster;
}

bool assaultblasterIsSingleItem(item *cItem){
	(void)cItem;

	return true;
}

bool assaultblasterHasMineAction(item *cItem){
	(void)cItem;

	return true;
}

bool assaultblasterMineAction(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	characterAddCooldown(cChar,15);
	beamblast(cChar,0.75f,0.2f,0.05f,1,1,8.f,1.f);
	return true;
}

bool assaultblasterActivateItem(item *cItem, character *cChar, int to){
	(void)cItem;

	if(to < 0){return false;}
	characterAddCooldown(cChar,64);
	beamblast(cChar,0.75f,0.2f,0.05f,1,3,8.f,1.f);
	return true;
}