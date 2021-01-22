#pragma once
#include "nujel.h"

char *lSDisplayVal(lVal *v, char *buf, char *bufEnd);
char *lSWriteVal  (lVal *v, char *buf, char *bufEnd);

lVal *lnfStrlen (lClosure *c, lVal *v);
lVal *lnfSubstr (lClosure *c, lVal *v);
lVal *lnfBr     (lClosure *c, lVal *v);
lVal *lnfCat    (lClosure *c, lVal *v);
lVal *lnfAnsiFG (lClosure *c, lVal *v);
lVal *lnfAnsiRS (lClosure *c, lVal *v);
