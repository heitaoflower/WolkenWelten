#pragma once
#include "../network/packet.h"

extern char chatLog[12][256];

void chatStartInput();
void chatCheckInput();
void chatParsePacket(packetLarge *p);
void chatPrintDebug(const char *msg);
void msgSendChatMessage(char *msg);
void msgSendDyingMessage(char *msg, int c);