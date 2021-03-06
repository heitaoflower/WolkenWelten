static const int ITEMID=273;

#include "../api_v1.h"

void clusterbombInit(){
	recipeNew3(itemNew(ITEMID,1), itemNew(I_Iron_Bar,3), itemNew(I_Coal, 12), itemNew(I_Crystal, 4));
	lispDefineID("i-","cluster bomb",ITEMID);
}

bool clusterbombSecondaryAction(item *cItem,character *cChar){
	if(characterTryToUse(cChar,cItem,200,1)){
		grenadeNew(vecAdd(cChar->pos,vecNew(0,0.5f,0)),cChar->rot,1,48,1.f);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,0,300);
		return true;
	}
	return false;
}

int clusterbombItemDropBurnUp(itemDrop *id){
	(void)id;
	if(id->ent == NULL){return 0;}
	explode(id->ent->pos, 4, 0);
	return 0;
}
