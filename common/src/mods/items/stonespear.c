static const int ITEMID=289;

#include "../api_v1.h"

void stonespearInit(){
	recipeNew2(itemNew(ITEMID,1), itemNew(I_Board,4), itemNew(I_Stone,4));
	lispDefineID("i-","stone spear",ITEMID);
}

bool stonespearPrimaryAction(item *cItem, character *cChar){
	if(characterIsAiming(cChar) && characterTryToUse(cChar,cItem,100,0)){
		throwableNew(cChar->pos, cChar->rot, 0.35f, *cItem, characterGetBeing(cChar), 4, THROWABLE_TIP_HEAVY | THROWABLE_PIERCE);
		characterAddRecoil(cChar,1.f);
		characterStopAim(cChar);
		itemDiscard(cItem);
		return true;
	}
	return false;
}

bool stonespearSecondaryAction(item *cItem, character *cChar){
	if(throwableTryAim(cItem,cChar)){return true;}
	return false;
}
