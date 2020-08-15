#include <ctype.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct itemProcedure;
typedef struct itemProcedure itemProcedure;
struct itemProcedure {
	char *name;
	char *type;
	char *prototype;
	itemProcedure *next;
};

typedef struct {
	int id;
	const char *filename;
	itemProcedure *proc;
} item;

item *items[512];
int itemCount=0;


void *loadFile(const char *filename,size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	uint8_t *buf = NULL;

	fp = fopen(filename,"rb");
	if(fp == NULL){return NULL;}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	buf = malloc(filelen);
	if(buf == NULL){return NULL;}

	readlen = 0;
	while(readlen < filelen){
		read = fread(buf+readlen,1,filelen-readlen,fp);
		if(read == 0){
			free(buf);
			return NULL;
		}
		readlen += read;
	}
	fclose(fp);

	*len = filelen;
	return buf;
}

void addItemProcedure(item *itm, itemProcedure *proc){
	itemProcedure *p;
	if(itm->proc == NULL){
		itm->proc = proc;
		return;
	}
	for(p = itm->proc;p->next != NULL;p = p->next){}
	p->next = proc;
}

void parseFunctionPrototype(const char *line, itemProcedure *proc){
	proc->prototype = strdup(line);
	for(char *s=proc->prototype;*s!=0;s++){
		if(*s == '{'){
			*s = ';';
			break;
		}
	}
}

void parseFunctionName(const char *line, itemProcedure *proc){
	int mode=0;
	
	proc->name = strdup(line);
	for(char *s=proc->name;*s!=0;s++){
		if(mode == 0){
			if(isspace(*s)){
				mode = 1;
			}
		}else if(mode == 1){
			if(!isspace(*s) && (*s != '*')){
				proc->name = s;
				break;
			}
		}
	}
	
	for(char *s=proc->name;*s!=0;s++){
		if(isspace(*s) || (*s == '(')){
			*s = 0;
			break;
		}
	}
	
	for(char *s=proc->name;*s!=0;s++){
		if(isupper(*s)){
			proc->type = s;
			break;
		}
	}
}


void parseLine(int lineNumber, const char *line, item *itm){
	if(line == NULL) {return;}
	if(*line == 0)   {return;}
	if(*line == ' ') {return;}
	if(*line == '\t'){return;}
	if(*line == '\r'){return;}
	if(*line == '\n'){return;}
	
	if(*line == '}') {return;}
	if(*line == '#') {return;}
	
	if(strncmp("static const int ITEMID=",line,24) == 0){
		itm->id = atoi(line+24);
		return;
	}
	
	itemProcedure *proc = calloc(1,sizeof(itemProcedure));
	parseFunctionPrototype(line,proc);
	parseFunctionName(line,proc);
	
	addItemProcedure(itm,proc);
}

void parseFile(const char *filename){
	size_t len = 0;
	item *itm;
	int lineNumber=0;
	unsigned char *file = loadFile(filename,&len);
	unsigned char *line = file;
	if(file == NULL){
		fprintf(stderr,"ERROR LOADING `%s`\n",filename);
		return;
	}
	itm = calloc(1,sizeof(item));
	itm->filename = filename;
	
	for(int i=0;i<len;i++){
		if(file[i] == '\n'){
			file[i] = 0;
			parseLine(lineNumber++,line,itm);
			line = file+i+1;
		}
	}
	
	items[itemCount++] = itm;
}

void printFunctionPrototypes(item *itm){
	itemProcedure *p;
	if(itm       == NULL){return;}
	for(p = itm->proc;p != NULL;p = p->next){
		puts(p->prototype);
	}
	puts("");
}

void printTypeCaller(const char *procName, const char *typeName){
	size_t tnLen = strlen(typeName);
	
	printf("void %s(){\n",procName);
	for(int i=0;i<itemCount;i++){
		item *itm = items[i];
		itemProcedure *p;
		if(itm       == NULL){continue;}
		for(p = itm->proc;p != NULL;p = p->next){
			if(strncmp(p->type,typeName,tnLen) == 0){
				printf("\t%s();\n",p->name);
			}
		}
	}
	puts("}");
}

void printItemTypeDispatch(const char *typeName, const char *argsAndTypes, const char *args, const char *retType){
	size_t tnLen = strlen(typeName);
	char *lcType = strdup(typeName);
	*lcType = tolower(*lcType);
	
	printf("%s%sDispatch(%s){\n",retType,lcType,argsAndTypes);
	puts("\tswitch(cItem->ID){");
	
	for(int i=0;i<itemCount;i++){
		item *itm = items[i];
		itemProcedure *p;
		if(itm       == NULL){continue;}
		for(p = itm->proc;p != NULL;p = p->next){
			if(strncmp(p->type,typeName,tnLen) == 0){
				printf("\t\tcase %i: return %s(%s);\n",itm->id,p->name,args);
			}
		}
	}
	
	puts("\t}");
	printf("\treturn %sDefault(%s);\n",lcType,args);
	puts("}");
	puts("");
}

int main(int argc, char *argv[]){
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	if(argc <= 1){
		fprintf(stderr,"USAGE: modscg [FILES]\n");
		return 1;
	}
	
	puts("#include \"../mods/mods.h\"\n");
	for(int i=1;i<argc;i++){
		parseFile(argv[i]);
	}
	
	for(int i=0;i<itemCount;i++){
		printFunctionPrototypes(items[i]);
	}
	
	printTypeCaller("modsInit","Init");
	printItemTypeDispatch("BlockDamage","item *cItem, blockCategory blockCat","cItem, blockCat","int ");
	printItemTypeDispatch("GetMesh","item *cItem","cItem","mesh *");
	printItemTypeDispatch("IsSingleItem","item *cItem","cItem","bool ");
	printItemTypeDispatch("HasMineAction","item *cItem","cItem","bool ");
	printItemTypeDispatch("MineAction","item *cItem, character *cChar, int to","cItem, cChar, to","bool ");
	printItemTypeDispatch("ActivateItem","item *cItem, character *cChar, int to","cItem, cChar, to","bool ");
	printItemTypeDispatch("GetInaccuracy","item *cItem","cItem","float ");
	
	return 0;
}