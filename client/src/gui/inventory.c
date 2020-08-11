#include "inventory.h"
#include "../main.h"
#include "../gfx/mesh.h"
#include "../game/item.h"
#include "../gfx/gfx.h"
#include "../sdl/sdl.h"
#include "../gui/gui.h"
#include "../sdl/sfx.h"
#include "../game/recipe.h"

#include <math.h>

const float ITEMTILE    = (1.f/32.f);
bool inventoryOpen      = false;
bool inventoryPickup    = false;
int  inventoryPickupSel = -1;
int  gamepadSelection   = -1;


void showInventory(){
	inventoryOpen = true;
	showMouseCursor();
}

void hideInventory(){
	inventoryOpen = false;
	hideMouseCursor();
}

bool isInventoryOpen(){
	return inventoryOpen;
}

void drawInventory(textMesh *guim){
	item *cItem;
	static int ticks = 0;
	unsigned char a;
	unsigned short b;
	int tilesize,itemtilesize,itemtilesizeoff;
	if(!isInventoryOpen()){return;}
	if(screenWidth < 1024){
		tilesize = 48;
		itemtilesize = 32;
	}else if(screenWidth < 1536){
		tilesize = 64;
		itemtilesize = 48;
	}else {
		tilesize = 80;
		itemtilesize = 64;
	}
	++ticks;
	itemtilesizeoff = (tilesize-itemtilesize)/2;
	guim->size = 2;

	int xraw = (mousex - (screenWidth/2-5*tilesize));
	int xsel = xraw/tilesize;
	int ysel = 4-(((mousey+tilesize) - (screenHeight/2))/tilesize);
	if(ysel == 0){ysel = -1;}
	if(mousey > ((screenHeight/2)+tilesize*4-tilesize/2)){
		ysel = 4-(((mousey+tilesize/2) - (screenHeight/2))/tilesize);
		if(ysel == 1){ ysel = -1;}
	}
	int sel = xsel + ysel*10;
	if((xsel < 0) || (xsel > 9) || (xraw < 0) || (ysel < 0) || (ysel > 59) || (mouseHidden)){
		sel = -1;
	}

	textMeshBox(guim,(screenWidth/2-5*tilesize)-tilesize/2,screenHeight/2-4*tilesize,11*tilesize,9*tilesize,23.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);

	guim->sx = (screenWidth/2-5*tilesize);
	guim->sy = screenHeight/2-4*tilesize+tilesize/2+tilesize/8;
	textMeshPrintf(guim,"Crafting");

	guim->sx = (screenWidth/2-5*tilesize);
	guim->sy = screenHeight/2-1*tilesize+tilesize/2+tilesize/8;
	textMeshPrintf(guim,"Inventory");

	for(int i = 0;i<40;i++){
		int x = (screenWidth/2-5*tilesize)+(i%10*tilesize);
		int y = screenHeight/2 + (4*tilesize) - (i/10*tilesize) - tilesize;
		if(i < 10){ y += tilesize/2; }
		if((i == sel) || (i == gamepadSelection) || (mouseHidden && (i == inventoryPickupSel))){
			textMeshBox(guim,x,y,tilesize,tilesize,21.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
		}else{
			textMeshBox(guim,x,y,tilesize,tilesize,20.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
		}
		cItem = &player->inventory[i];
		if(cItem == NULL){continue;}
		b = cItem->ID;
		a = cItem->amount;
		if(a == 0){continue;}
		if((inventoryPickupSel != i) || (mouseHidden)){
			int u = b % 32;
			int v = b / 32;
			textMeshBox(guim,x+itemtilesizeoff,y+itemtilesizeoff,itemtilesize,itemtilesize,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);
			if(!itemIsSingle(cItem)){
				guim->sx = x+tilesize-tilesize/4;
				guim->sy = y+(itemtilesize-itemtilesizeoff)+tilesize/32;
				textMeshNumber(guim,a);
			}
		}
	}


	for(int i=0;i<10;i++){
		const int y = screenHeight/2 - tilesize*3;
		const int x = (screenWidth/2)+((i-5)*tilesize);
		if(i >= recipeGetCount()){break;}
		int r = i;
		b = recipeGetResultID(r);
		a = recipeCanCraft(r,player);
		int u = b % 32;
		int v = b / 32;
		if((i == (sel-50)) || (i == (gamepadSelection-50))){
			textMeshBox(guim,x,y,tilesize,tilesize,21.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
		}else if(a <= 0){
			textMeshBox(guim,x,y,tilesize,tilesize,22.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
		}else{
			textMeshBox(guim,x,y,tilesize,tilesize,20.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
		}

		textMeshBox(guim,x+itemtilesizeoff,y+itemtilesizeoff,itemtilesize,itemtilesize,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);

		guim->sx = x+tilesize-tilesize/3;
		guim->sy = y+(itemtilesize-itemtilesizeoff);
		textMeshNumber(guim,a);

		if(((mousex > x) && (mousex < x+tilesize) && (mousey > y) && (mousey < y+tilesize)) || (mouseHidden && (i == (gamepadSelection-50)))){
			const int yy = y + tilesize + tilesize/2;
			int ii,xx;
			const int animX = sin((float)ticks/16.f)*tilesize/8;
			const int animY = cos((float)ticks/16.f)*tilesize/8;

			for(ii=0;ii<4;ii++){
				xx = (screenWidth/2)+((ii*2-5)*tilesize);
				b = recipeGetIngredientID(r,ii);
				a = recipeGetIngredientAmount(r,ii);
				if((b == 0) || (a <= 0)){ break;}
				b = ingredientSubstituteGetSub(b,(ticks/96) % (ingredientSubstituteGetAmount(b)+1));
				
				u = b % 32;
				v = b / 32;
				if(ii > 0){
					textMeshBox(guim,xx-tilesize+tilesize/4+animX/2,yy+tilesize/4+animY/2,tilesize/2,tilesize/2,24.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
				}
				textMeshBox(guim,xx+itemtilesizeoff,yy+itemtilesizeoff,itemtilesize,itemtilesize,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);
				guim->sx = xx+tilesize-tilesize/3;
				guim->sy = yy+(itemtilesize-itemtilesizeoff);
				textMeshNumber(guim,a);
			}
			b = recipeGetResultID(r);
			u = b % 32;
			v = b / 32;
			xx = (screenWidth/2)+((ii*2-5)*tilesize);
			textMeshBox(guim,xx-tilesize/2-tilesize/4+animY,yy+tilesize/2-tilesize/4,tilesize/2,tilesize/2,25.f/32.f,31.f/32.f,1.f/32.f,1.f/32.f,~1);
			textMeshBox(guim,xx+itemtilesizeoff,yy+itemtilesizeoff,itemtilesize,itemtilesize,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);

			guim->sx = xx+tilesize-tilesize/3;
			guim->sy = yy+(itemtilesize-itemtilesizeoff);
			textMeshNumber(guim,recipeGetResultAmount(r));
		}
	}

	if((inventoryPickup) &&  (!mouseHidden)){
		cItem = characterGetItemBarSlot(player,inventoryPickupSel);
		if(cItem == NULL){return;}
		b = cItem->ID;
		a = cItem->amount;
		if(a == 0){return;}
		int u = b % 32;
		int v = b / 32;
		textMeshBox(guim,mousex,mousey,itemtilesize,itemtilesize,u*ITEMTILE,v*ITEMTILE,1.f/32.f,1.f/32.f,~1);
		if(!itemIsSingle(cItem)){
			guim->sx = mousex+tilesize-tilesize/3;
			guim->sy = mousey+(itemtilesize-itemtilesizeoff);
			textMeshNumber(guim,a);
		}
	}
}

void inventoryClickOutside(int btn){
	if((btn == 1) && inventoryPickup){
		inventoryPickup = false;
		characterDropItem(player,inventoryPickupSel);
		inventoryPickupSel = -1;
	}
}

void updateInventoryClick(int x,int y, int btn){
	item *cItem;
	int tilesize;
	if(!isInventoryOpen()){return;}
	
	if(screenWidth < 1024){
		tilesize = 48;
	}else if(screenWidth < 1536){
		tilesize = 64;
	}else {
		tilesize = 80;
	}
	if(x < ((screenWidth /2)-(5*tilesize)))           {inventoryClickOutside(btn);return;}
	if(x > ((screenWidth /2)+(5*tilesize)))           {inventoryClickOutside(btn);return;}
	if(y < ((screenHeight/2)-(3*tilesize)))           {inventoryClickOutside(btn);return;}
	if(y > ((screenHeight/2)+(4*tilesize)+tilesize/2)){inventoryClickOutside(btn);return;}

	int xsel = (x - (screenWidth/2-5*tilesize))/tilesize;
	int ysel = 4-(((y+tilesize) - (screenHeight/2))/tilesize);
	if(ysel == 0){ysel = -1;}
	if(y > ((screenHeight/2)+tilesize*4-tilesize/2)){
		ysel = 4-(((y+tilesize/2) - (screenHeight/2))/tilesize);
		if(ysel == 1){ inventoryClickOutside(btn);return;}
	}
	int sel = xsel + ysel*10;
	if(ysel == 4){return;}
	if(ysel == 5){
		int r = xsel;
		if(btn == 1){
			recipeDoCraft(r,player,1);
		}else if(btn == 3){
			recipeDoCraft(r,player,recipeCanCraft(r,player));
		}
		sfxPlay(sfxPock,1.f);
		return;
	}

	if(btn == 1){
		if(!inventoryPickup){
			cItem = characterGetItemBarSlot(player,sel);
			if(cItem == NULL){return;}
			inventoryPickup = true;
			inventoryPickupSel = sel;
		}else{
			inventoryPickup = false;
			characterSwapItemSlots(player,inventoryPickupSel,sel);
			inventoryPickupSel = -1;
		}
		sfxPlay(sfxPock,1.f);
	}
	if(btn == 3){
		cItem = characterGetItemBarSlot(player,sel);
		if(cItem == NULL){return;}
		characterDropItem(player,sel);
		sfxPlay(sfxPock,1.f);
	}
}

void changeInventorySelection(int x,int y){
	mouseHidden = true;
	if(gamepadSelection == -1){
		gamepadSelection = 0;
		return;
	}
	int xsel = gamepadSelection % 10;
	int ysel = gamepadSelection / 10;

	int xmax = 9;
	if(ysel == 5){xmax = MIN(9,recipeGetCount()-1);}
	xsel += x;
	if(xsel < 0){xsel = xmax;}
	if(xsel > xmax){xsel = 0;}
	ysel += y;
	if(ysel < 0){ysel = 5;}
	if(ysel == 4){
		if(y > 0){
			ysel = 5;
		}else{
			ysel = 3;
		}
	}
	if(ysel > 5){ysel = 0;}
	gamepadSelection = xsel + ysel*10;
}

void updateInventoryGamepad(int btn){
	mouseHidden = true;
	if(gamepadSelection == -1){
		gamepadSelection = 0;
		return;
	}

	if((gamepadSelection >= 50) && (gamepadSelection < 60)){
		int r = gamepadSelection-50;
		if(btn == 1){
			recipeDoCraft(r,player,1);
		}else if(btn == 3){
			recipeDoCraft(r,player,recipeCanCraft(r,player));
		}
	}else if((gamepadSelection >= 0) && (gamepadSelection < 40)){
		if(btn == 1){
			if(!inventoryPickup){
				item *cItem = characterGetItemBarSlot(player,gamepadSelection);
				if(cItem == NULL){return;}
				inventoryPickup = true;
				inventoryPickupSel = gamepadSelection;
			}else{
				inventoryPickup = false;
				characterSwapItemSlots(player,inventoryPickupSel,gamepadSelection);
				inventoryPickupSel = -1;
			}
		}else if(btn == 3){
			item *cItem = characterGetItemBarSlot(player,gamepadSelection);
			if(cItem == NULL){return;}
			characterDropItem(player,gamepadSelection);
		}
	}
}
