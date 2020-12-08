#include "lisp.h"

#include "../gui/gui.h"
#include "../../../common/src/network/messages.h"
#include "../../../common/nujel/nujel.h"

#include <stdio.h>
#include <string.h>

lClosure *clRoot;
u8 SEvalID;

lVal *lispSEvalSym(u8 id){
	static char buf[8];
	snprintf(buf,sizeof(buf)-1,"SEv%03u",id);
	return lValSym(buf);
}

void lispEvalNR(const char *str){
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		lEval(clRoot,sexpr);
	}
}

lVal *wwlnfSEval(lClosure *c, lVal *v){
	(void)c;
	static char buf[256];
	lSPrintChain(v,buf,&buf[sizeof(buf)-1]);
	if(++SEvalID == 0){++SEvalID;}
	msgLispSExpr(-1,SEvalID,buf);
	return lispSEvalSym(SEvalID);
}

void lispInit(){
	lInit();
	clRoot = lClosureNew(NULL);
	lispEvalNR("(define abs (lambda (a) (cond ((< a 0) (- 0 a)) (#t a))))");
	lispEvalNR("(define test (lambda (a) (s-eval (water (px) (py) (pz)))))");
	lClosureAddNF(clRoot,"s-eval", &wwlnfSEval);
}
void lispFree(){
	lClosureFree(clRoot);
}

const char *lispEval(const char *str){
	static char reply[512];
	memset(reply,0,512);
	lVal *v = NULL;
	for(lVal *sexpr = lParseSExprCS(str); sexpr != NULL; sexpr = sexpr->next){
		v = lEval(clRoot,sexpr);
	}
	lSPrintChain(v,reply,&reply[sizeof(reply)-1]);
	for(uint i=0;i<sizeof(reply);i++){if(reply[i] == '\n'){reply[i] = ' ';}}
	lClosureGC();
	return reply;
}

void lispRecvSExpr(const packet *p){
	u8 id = p->v.u8[0];
	const char *str = (const char *)&p->v.u8[1];
	lispPanelShowReply(lispSEvalSym(id),str);
}