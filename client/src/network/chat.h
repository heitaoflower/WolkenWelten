#pragma once
#include "../../../common/src/network/packet.h"

extern char chatLog[12][256];

void        chatEmpty           ();
void        chatParsePacket     (const packet *p);
void        chatPrintDebug      (const char *msg);
void        msgSendChatMessage  (const char *msg);
void        msgSendDyingMessage (const char *msg, int c);
const char *chatGetPrevHistory  ();
const char *chatGetNextHistory  ();
void        chatResetHistorySel ();
