static const int ITEMID=261;

#include "../api_v1.h"

void blasterInit(){
	(void)ITEMID;
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Crystal_Bar,4), itemNew(I_Iron_Bar,4), itemNew(I_Crystalbullet,4));
	lispDefineID("i-","blaster",ITEMID);
}

bool blasterPrimaryAction(item *cItem, character *cChar){
	if(throwableTry(cItem,cChar,0.1, 1, THROWABLE_PITCH_SPIN)){return true;}
	if(!characterTryToShoot(cChar,cItem,80,3)){return false;}
	beamblast(cChar,1.f,8.0f,0.05f,3,1,2.f,1.f);
	return true;
}

bool blasterSecondaryAction(item *cItem, character *cChar){
	if(characterTryToUse(cChar,cItem,200,0)){
		characterAddCooldown(cChar,200);
		characterToggleAim(cChar,4.f);
		characterAddInaccuracy(cChar,32.f);
		return true;
	}
	return false;
}

bool blasterTertiaryAction(item *cItem, character *cChar){
	return characterItemReload(cChar, cItem, 50);
}

int blasterItemDropBurnUp(itemDrop *id){
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 0.2f*id->itm.amount, 0);
	return 0;
}
