static const int ITEMID=277;

#include "../api_v1.h"

void poopInit(){
	lispDefineID("i-","poop",ITEMID);
}

bool poopSecondaryAction(item *cItem,character *cChar){
	if(characterGetHP(cChar) >= characterGetMaxHP(cChar)){return false;}
	if(characterTryToUse(cChar,cItem,200,1)){
		characterHP(cChar,1);
		characterAddCooldown(cChar,200);
		characterStartAnimation(cChar,4,600);
		sfxPlay(sfxChomp,1.f);
		return true;
	}
	return false;
}

int poopItemDropCallback(const item *cItem, float x, float y, float z){
	(void)cItem;
	uint cx = x;
	uint cy = y-0.5f;
	uint cz = z;
	u8 b = worldGetB(cx,cy,cz);
	if(b == 0){return 0;}
	if(b == I_Grass){
		if(rngValA(1023) != 0){return 0;}
		switch(rngValA(31)){
		default:
			return -1;
		case 0:
			vegShrub(cx,cy,cz);
			return -cItem->amount;
		case 1:
			vegSpruce(cx,cy,cz);
			return -cItem->amount;
		case 2:
			vegOak(cx,cy,cz);
			return -cItem->amount;
		case 3:
			vegSpruce(cx,cy,cz);
			return -cItem->amount;
		}
	}else if(b == I_Dirt){
		if(rngValM(255) != 0){return 0;}
		worldSetB(x,y-0.5f,z,I_Grass);
		return -1;
	}else{
		if(rngValM(1023) == 0){
			return -1;
		}
		return 0;
	}
}
