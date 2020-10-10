#define _GNU_SOURCE

#include "client.h"
#include "../main.h"
#include "../gfx/effects.h"
#include "../gui/menu.h"
#include "../game/animal.h"
#include "../game/blockMining.h"
#include "../game/character.h"
#include "../game/grenade.h"
#include "../game/itemDrop.h"
#include "../misc/options.h"
#include "../network/chat.h"
#include "../../../common/src/misc/lz4.h"
#include "../../../common/src/misc/misc.h"
#include "../../../common/src/network/messages.h"
#include "../voxel/bigchungus.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int recvBufLen = 0;
u8 recvBuf[1<<20];

int sendBufSent = 0;
int sendBufLen  = 0;
u8 sendBuf[1<<16];

size_t sentBytesCurrentSession             = 0;
size_t recvBytesCurrentSession             = 0;
size_t recvUncompressedBytesCurrentSession = 0;

int serverPort        = 6309;
pid_t singlePlayerPID = 0;
int connectionTries   = 0;

#ifdef __EMSCRIPTEN__
#include "client_wasm.h"
#elif defined __MINGW32__
#include "client_win.h"
#else
#include "client_bsd.h"
#endif

void msgParseGetChunk(const packet *p){
	int x = p->val.i[1024];
	int y = p->val.i[1025];
	int z = p->val.i[1026];
	chungus *chng =  worldGetChungus(x>>8,y>>8,z>>8);
	if(chng == NULL){return;}
	chunk *chnk = chungusGetChunkOrNew(chng,x,y,z);
	if(chnk == NULL){return;}
	memcpy(chnk->data,p->val.c,sizeof(chnk->data));
	chnk->ready &= ~15;
}

void msgSendPlayerPos(){
	static int inventoryCountDown=0;
	if(player == NULL){return;}
	if(--inventoryCountDown < 0){
		msgPlayerSetInventory(-1,player->inventory,40);
		inventoryCountDown = 16;
	}
	packet *p = &packetBuffer;

	p->val.f[ 0] = player->pos.x;
	p->val.f[ 1] = player->pos.y;
	p->val.f[ 2] = player->pos.z;
	p->val.f[ 3] = player->rot.yaw;
	p->val.f[ 4] = player->rot.pitch;
	p->val.f[ 5] = player->rot.roll;
	p->val.f[ 6] = player->vel.x;
	p->val.f[ 7] = player->vel.y;
	p->val.f[ 8] = player->vel.z;
	p->val.f[ 9] = player->yoff;

	if(player->hook != NULL){
		p->val.i[10] = 1;
		p->val.f[11] = player->hook->ent->pos.x;
		p->val.f[12] = player->hook->ent->pos.y;
		p->val.f[13] = player->hook->ent->pos.z;
	} else {
		p->val.i[10] = 0;
	}
	p->val.i[14] = player->blockMiningX;
	p->val.i[15] = player->blockMiningY;
	p->val.i[16] = player->blockMiningZ;
	p->val.i[17] = player->activeItem;
	p->val.i[18] = player->animationIndex;
	p->val.i[20] = player->animationTicksMax;
	p->val.i[21] = player->animationTicksLeft;
	p->val.i[22] = player->flags;
	p->val.i[23] = player->hp;

	packetQueueToServer(p,15,24*4);
}

void decompressPacket(const packet *p){
	u8 *t;
	u8 buf[1<<20];
	int len = LZ4_decompress_safe((const char *)&p->val.c, (char *)buf, packetLen(p), sizeof(buf));
	if(len <= 0){
		fprintf(stderr,"Decompression return %i\n",len);
		exit(1);
	}
	for(t=buf;(t-buf)<len;t+=alignedLen(packetLen((packet *)t)) + 4){
		clientParsePacket((packet *)t);
	}
}

void clientParsePacket(const packet *p){
	const int pLen  = packetLen(p);
	const int pType = packetType(p);
	if(pType != 0xFF){
		recvUncompressedBytesCurrentSession += pLen+4;
	}

	switch(pType){
		case 0: // Keepalive
			break;
		case 1: // playerPos
			characterSetPos(player,vecNewP(&p->val.f[0]));
			characterSetRot(player,vecNewP(&p->val.f[3]));
			characterSetVelocity(player,vecZero());
			characterFreeHook(player);
			player->flags &= ~CHAR_SPAWNING;
			break;
		case 2: // requestChungus
			fprintf(stderr,"Received a requestChungus packet from the server which should never happen.\n");
			break;
		case 3: // placeBlock
			worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],p->val.i[3]);
			break;
		case 4: // mineBlock
			if(worldSetB(p->val.i[0],p->val.i[1],p->val.i[2],0)){
				fxBlockBreak(vecNew(p->val.i[0],p->val.i[1],p->val.i[2]),p->val.i[3]);
			}
			break;
		case 5: // Goodbye
			fprintf(stderr,"Received a Goodbye packet from the server which should never happen.\n");
			break;
		case 6: // blockMiningUpdate
			blockMiningUpdateFromServer(p);
			break;
		case 7:
			worldSetChungusLoaded(p->val.i[0],p->val.i[1],p->val.i[2]);
			break;
		case 8:
			characterGotHitBroadcast(p->val.i[1],p->val.i[0]);
			break;
		case 9:
			fprintf(stderr,"Received a PlayerJoin packet from the server which should never happen.\n");
			break;
		case 10:
			fprintf(stderr,"Received an itemDropNew msg from the server, this should never happen.\n");
			break;
		case 11:
			fprintf(stderr,"Received a grenadeNew packet from the server which should never happen.\n");
			break;
		case 12:
			fprintf(stderr,"Received a beamblast packet from the server which should never happen.\n");
			break;
		case 13: // playerMoveDelta
			characterMoveDelta(player,p);
			break;
		case 14: // characterName
			characterSetName(p);
			break;
		case 15: // playerPos
			characterSetPlayerPos(p);
			break;
		case 16: // chatMsg
			chatParsePacket(p);
			break;
		case 17: // dyingMsg
			fprintf(stderr,"Received a dying message packet from the server which should never happen.\n");
			break;
		case 18: // chunkData
			msgParseGetChunk(p);
			break;
		case 19: // setPlayerCount
			characterRemovePlayer(p->val.u[1],p->val.u[0]);
			break;
		case 20: // playerPickupItem
			characterPickupItem(player,p->val.s[0],p->val.s[1]);
			break;
		case 21: // itemDropUpdate
			fprintf(stderr,"Received an itemDropDel msg from the server, this should never happen.\n");
			break;
		case 22: // grenadeExplode
			grenadeExplode(vecNewP(&p->val.f[0]),p->val.f[3],p->val.i[4]);
			break;
		case 23: // grenadeUpdate
			grenadeUpdateFromServer(p);
			break;
		case 24: // fxBeamBlaster
			fxBeamBlaster(vecNewP(&p->val.f[0]),vecNewP(&p->val.f[3]),p->val.f[6],p->val.f[7],p->val.f[8],p->val.i[9],p->val.i[10]);
			break;
		case 25: // msgItemDropUpdate
			itemDropUpdateFromServer(p);
			break;
		case 26: // msgPlayerDamage
			characterDamagePacket(player,p);
			break;
		case 28:
			characterSetData(player,p);
			break;
		case 29:
			characterSetInventoryP(player,p);
			break;
		case 30:
			animalSyncFromServer(p);
			break;
		case 31:
			fprintf(stderr,"Received an dirtyChunk msg from the server, this should never happen.\n");
			break;
		case 32:
			fprintf(stderr,"Received an animalDmg msg from the server, this should never happen.\n");
			break;
		case 0xFF: // compressedMultiPacket
			decompressPacket(p);
			break;
		default:
			fprintf(stderr,"%i[%i] UNKNOWN PACKET\n",pType,pLen);
			break;
	}
}

void clientParse(){
	int off=0;
	if(recvBufLen == 0){return;}

	for(int max=128;max > 0;--max){
		if(off >= recvBufLen){break;}
		int pLen = packetLen((packet *)(recvBuf+off));
		if((pLen+4+off) > recvBufLen){
			break;
		}
		clientParsePacket((packet *)(recvBuf+off));
		fflush(stderr);
		off += alignedLen(pLen) + 4;
	}
	if(off < recvBufLen){
		for(int i=0;i<recvBufLen-off;i++){
			recvBuf[i] = recvBuf[i+off];
		}
	}
	recvBufLen -= off;
}

void clientSendIntroduction(){
	char introStr[64];
	#ifndef __EMSCRIPTEN__
	queueToServer("NATIVE\r\n\r\n",10);
	#endif
	uint len = snprintf(introStr,sizeof(introStr),"%.32s\n",playerName);
	queueToServer(introStr,len);
	msgNOP(2048);
	clientSendAllToServer();
}

void clientGreetServer(){
	clientSendIntroduction();
}

void clientHandleEvents(){
	clientRead();
	clientParse();
	msgSendPlayerPos();
}

void clientGoodbye(){
	if(serverSocket <= 0){return;}
	msgGoodbye();
	clientSendAllToServer();
}

void queueToServer(const void *data, uint len){
	if((sendBufLen + len) > sizeof(sendBuf)){
		clientSendAllToServer();
	}
	memcpy(sendBuf + sendBufLen, data, len);
	sendBufLen += len;
}

void clientFree(){
	clientFreeSpecific();
	menuSetError("Connection closed");
	singlePlayerPID=0;
	recvBufLen = 0;
	sendBufLen = 0;
	sendBufSent = 0;
	bigchungusFree(&world);
	characterInit(player);
	chatEmpty();
}
