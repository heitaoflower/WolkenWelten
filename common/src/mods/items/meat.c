static const int ITEMID=278;

#include "../api_v1.h"

void meatInit(){
	lispDefineID("i-","meat raw",ITEMID);
}

bool meatSecondaryAction(item *cItem,character *cChar){
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(characterTryToUse(cChar,cItem,200,1)){
		characterHP(cChar,2);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}

int meatItemDropCallback(const item *cItem, float x, float y, float z){
	(void)cItem;
	(void)x;
	(void)y;
	(void)z;
	if(rngValM(1<<15) == 0){ return -1;}
	return 0;
}

int meatItemDropBurnUpCallback(itemDrop *id){
	id->itm.ID = I_Cookedmeat;
	return 1;
}
