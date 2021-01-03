#include "mainmenu.h"

#include "../main.h"
#include "../gui/gui.h"
#include "../gui/menu.h"
#include "../gui/textInput.h"
#include "../gui/widget.h"
#include "../menu/singleplayer.h"
#include "../menu/multiplayer.h"
#include "../menu/options.h"
#include "../misc/options.h"
#include "../../../common/src/tmp/cto.h"
#include "../../../common/src/misc/misc.h"

#include <stdio.h>
#include <string.h>

widget *mainMenu;

widget *buttonQuit;
widget *buttonReturnToGame;
widget *buttonExitToMenu;

static void handlerSingleplayer(widget *wid){
	(void)wid;
	openSingleplayerMenu();
}
static void handlerMultiplayer(widget *wid){
	(void)wid;
	openMultiplayerMenu();
}

static void handlerOptions(widget *wid){
	(void)wid;
	openOptionsMenu();

}
static void handlerAttribution(widget *wid){
	(void)wid;
	closeAllMenus();
	openAttributions();
	widgetFocus(NULL);
}

static void handlerQuit(widget *wid){
	(void)wid;
	quit = true;
}

static void handlerReturnToGame(widget *wid){
	(void)wid;
	if(!gameRunning){return;}
	closeAllMenus();
	widgetFocus(widgetGameScreen);
}

static void handlerExitToMenu(widget *wid){
	(void)wid;
	if(!gameRunning){return;}
	menuCloseGame();
	openMainMenu();
}

void initMainMenu(){
	mainMenu = widgetNewCP(wPanel,rootMenu,-1,0,0,-1);
	widgetNewCP  (wSpace ,mainMenu,16,0,256,0);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Singleplayer","click",handlerSingleplayer);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Multiplayer","click",handlerMultiplayer);
	widgetNewCP  (wHR ,mainMenu,16,0,256,32);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Options","click",handlerOptions);
	widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Attribution","click",handlerAttribution);
	widgetNewCP  (wHR ,mainMenu,16,0,256,32);
	buttonQuit         = widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Quit","click",handlerQuit);
	buttonExitToMenu   = widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Exit to Menu","click",handlerExitToMenu);
	buttonReturnToGame = widgetNewCPLH(wButton,mainMenu,16,0,256,32,"Return to Game","click",handlerReturnToGame);

	widgetLayVert(mainMenu,16);
}

void openMainMenu(){
	closeAllMenus();
	if(gameRunning){
		buttonQuit->flags         |=  WIDGET_HIDDEN;
		buttonExitToMenu->flags   &= ~WIDGET_HIDDEN;
		buttonReturnToGame->flags &= ~WIDGET_HIDDEN;
	}else{
		buttonQuit->flags         &= ~WIDGET_HIDDEN;
		buttonExitToMenu->flags   |=  WIDGET_HIDDEN;
		buttonReturnToGame->flags |=  WIDGET_HIDDEN;
	}
	widgetSlideW(mainMenu, 288);
	mainMenu->gw = 288;
}

void closeMainMenu(){
	widgetSlideW(mainMenu, 0);
}
