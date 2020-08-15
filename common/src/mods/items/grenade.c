static const int ITEMID=256;

#include "../api_v1.h"

void grenadeInit(){
	recipeAdd2I(ITEMID,2, 4,1, 13,1); // Coal(1) + Hematite Ore(1) -> Grenade(1)
}

bool grenadeActivateItem(item *cItem,character *cChar, int to){
	(void)cItem;
	if(to < 0){return false;}
	if(itemDecStack(cItem,1)){
		grenadeNew(cChar,1);
		characterAddCooldown(cChar,200);
		return true;
	}
	return false;
}

mesh *grenadeGetMesh(item *cItem){
	(void)cItem;

	return meshGrenade;
}