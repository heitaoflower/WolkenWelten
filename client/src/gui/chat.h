#pragma once
#include "../../../common/src/common.h"
#include "widget.h"

extern widget *chatPanel,*chatText;

void chatClose();
void chatOpen ();
void chatInit ();
void chatDraw (textMesh *guim);
