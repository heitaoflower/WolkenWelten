#include "main.h"

#include "misc/options.h"
#include "game/animal.h"
#include "game/blockMining.h"
#include "game/itemDrop.h"
#include "game/grenade.h"
#include "persistence/savegame.h"
#include "voxel/bigchungus.h"
#include "voxel/chungus.h"
#include "voxel/chunk.h"
#include "network/server.h"
#include "../../common/src/tmp/cto.h"
#include "../../common/src/misc/misc.h"
#include "../../common/src/game/blockType.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>

int playerID  = 64;
bool isClient = false;
bool quit = false;
char *termColors[16];
char *termReset;
char *nop = "";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

u64 getTicks(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (tv.tv_usec / 1000) + (tv.tv_sec * 1000);
}

void signalQuit(int signo){
	(void)signo;
	quit = true;
}

void initSignals(){
	#ifdef __MINGW32__
	signal(SIGTERM, signalQuit);
	signal(SIGABRT, signalQuit);
	#else
	signal(SIGPIPE, SIG_IGN);
	signal(SIGKILL, signalQuit);
	signal(SIGSTOP, signalQuit);
	signal(SIGHUP,  signalQuit);
	#endif
	signal(SIGINT,  signalQuit);
}

void initTermColors(){
	char *clicolor = getenv("CLICOLOR");
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	if((clicolor == NULL) || (*clicolor == 0) || (atoi(clicolor) == 0)){
		termReset = "";
		for(int i=0;i<16;i++){
			termColors[i] = nop;
		}
	}else{
		termReset = "\033[0m";
		for(int i=0;i<16;i++){
			termColors[i] = ansiFG[i];
		}
	}
}

void updateWorldStep(){
	blockMiningUpdate();
	itemDropUpdate();
	grenadeUpdate();
	animalUpdateAll();
}

void updateWorld(){
	static u64 lastUpdate  = 0;
	static u64 lastThought = 0;
	static u64 lastNeeds   = 0;
	int i = 0;
	const u64 cTicks = getTicks();
	if(lastUpdate  == 0){lastUpdate  = getTicks() -    5;}
	if(lastThought == 0){lastThought = getTicks() -  100;}
	if(lastNeeds   == 0){lastNeeds   = getTicks() - 1000;}

	i = 5;
	for(;lastUpdate +  5 < cTicks;lastUpdate +=  5){
		updateWorldStep();
		if(--i == 0){break;}
	}

	i = 1;
	for(;lastThought+100 < cTicks;lastThought += 100){
		animalThinkAll();
		if(--i == 0){break;}
	}

	for(;lastNeeds+5000 < cTicks;lastNeeds += 5000){
		animalNeedsAll();
	}
}

void handleAnimalPriorities(){
	static u64 lastCall   = 0;
	static uint c = 0;
	const u64 cTicks = getTicks();
	if(cTicks < lastCall + 200) {return;}
	c = (c+1) & 0x1F;
	if(clients[c].state)        {return;}
	if(clients[c].c == NULL)    {return;}
	lastCall = cTicks;
	animalUpdatePriorities(c);
}

int main( int argc, const char* argv[] ){
	initSignals();
	initTermColors();
	initOptions(argc,argv);
	savegameLoad();
	seedRNG(time(NULL));
	serverInit();
	chunkInit();
	savegameSave();

	printf("%sWolkenwelten",termColors[2]                );
	printf(" %s%s"         ,termColors[6],VERSION        );
	printf(" %s[%.16s]"    ,termColors[3],COMMIT         );
	printf(" %sSeed[%u]"   ,termColors[4],optionWorldSeed);
	printf(" %s{%s}"       ,termColors[5],optionSavegame );
	printf(" %sbuilt %s\n" ,termReset    ,BUILDDATE      );

	bigchungusInit(&world);
	blockTypeInit();
	bigchungusGenSpawn(&world);
	while(!quit){
		freeTime = getTicks();
		chungusUnsubFarChungi();
		chungusFreeOldChungi(30000);
		handleAnimalPriorities();
		bigchungusSafeSave(&world);
		updateWorld();
		serverHandleEvents();
		if(clientCount == 0){
			usleep(100000);
		}else{
			usleep(1000);
		}
	}
	printf("Exiting cleanly\n");
	savegameSave();
	serverFree();
	return 0;
}
