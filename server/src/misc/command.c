#define _DEFAULT_SOURCE
#include "command.h"

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../network/server.h"
#include "../../../common/src/network/messages.h"

char replyBuf[256];

static void cmdDmg(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	if((cmdLen > 3) && (cmd[3] == ' ')){
		int target = getClientByName(cmd+4);
		if(target >= 0){
			msgPlayerDamage(target,1);
		}else{
			snprintf(replyBuf,sizeof(replyBuf),".dmg : Can't find '%s'\n",cmd+4);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
		return;
	}
	msgPlayerDamage(c,1);
}

static void cmdDie(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	if((cmdLen > 3) && (cmd[3] == ' ')){
		int target = getClientByName(cmd+4);
		if(target >= 0){
			msgPlayerDamage(target,1000);
		}else{
			snprintf(replyBuf,sizeof(replyBuf),".die : Can't find '%s'\n",cmd+4);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
		return;
	}
	msgPlayerDamage(c,1000);
}

static void cmdHeal(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	if((cmdLen > 4) && (cmd[4] == ' ')){
		int target = getClientByName(cmd+5);
		if(target >= 0){
			msgPlayerDamage(target,-1000);
		}else{
			snprintf(replyBuf,sizeof(replyBuf),".heal : Can't find '%s'\n",cmd+5);
			replyBuf[sizeof(replyBuf)-1]=0;
			serverSendChatMsg(replyBuf);
		}
		return;
	}
	msgPlayerDamage(c,-1000);
}

static void cmdGive(int c, const char *cmd){
	int cmdLen = strnlen(cmd,252);
	if((cmdLen > 4) && (cmd[4] == ' ') && (isdigit(cmd[5]))){
		int target = atoi(cmd+5);
		if(target > 0){
			msgPickupItem(c,target,1);
			return;
		}
	}
	snprintf(replyBuf,sizeof(replyBuf),".give : You have to type in an ItemID\n");
	replyBuf[sizeof(replyBuf)-1]=0;
	serverSendChatMsg(replyBuf);
}

int parseCommand(int c, const char *cmd){
	if(cmd[0] != '.'){return 0;}
	const char *tcmp = cmd+1;
	
	if(strncmp(tcmp,"dmg",3) == 0){
		cmdDmg(c,tcmp);
		return 1;
	}
	
	if(strncmp(tcmp,"die",3) == 0){
		cmdDie(c,tcmp);
		return 1;
	}
	
	if(strncmp(tcmp,"heal",4) == 0){
		cmdHeal(c,tcmp);
		return 1;
	}
	
	if(strncmp(tcmp,"give",4) == 0){
		cmdGive(c,tcmp);
		return 1;
	}
		
	fprintf(stderr,"CMD [%s]\n",cmd);
	return 1;
}