#include "../gfx/texture.h"
#include "../game/blockType.h"
#include "../misc/options.h"
#include "../../../common/src/misc/misc.h"
#include "../tmp/assets.h"

#include "../gfx/gl.h"
#include "../gfx/lodepng.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

unsigned int boundTexture = 0;
uint textureCount         = 0;
texture textureList[32];

texture *tBlocks;
texture *tGui;
texture *tCursor;
texture *tItems;
texture *tCrosshair;
texture *tRope;
texture *tBlockMining;

void textureLoadSurface(texture *t, uint w, uint h, const void *data){
	t->w = w;
	t->h = h;
	if(t->ID == 0){
		glGenTextures(1, &t->ID);
	}
	glBindTexture(GL_TEXTURE_2D, t->ID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
}

void textureLoad(texture *t, const u8 *data, const size_t dataLen){
	unsigned char *pixels = NULL;
	int error = lodepng_decode32(&pixels, &t->w, &t->h, data, dataLen);
	if(error){
		fprintf(stderr,"Error decoding PNG\n");
		exit(4);
	}
	if(t->ID == 0){
		glGenTextures(1, &t->ID);
	}
	glBindTexture(GL_TEXTURE_2D, t->ID);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, t->w, t->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	free(pixels);
}

texture *textureNew(const u8 *data, size_t dataLen,const char *filename){
	texture *tex = &textureList[textureCount++];
	tex->ID = 0;
	tex->filename = filename;
	tex->modTime  = time(NULL);
	textureLoad(tex,data,dataLen);
	return tex;
}

texture *textureNewSurface(uint w, uint h, const void *data){
	texture *tex = &textureList[textureCount++];
	tex->ID = 0;
	textureLoadSurface(tex,w,h,data);
	return tex;
}

void textureFree(){
	for(uint i=0;i<textureCount;i++){
		if(textureList[i].ID == 0){continue;}
		glDeleteTextures(1, &textureList[i].ID);
	}
}

void textureBind(const texture *tex){
	if(boundTexture == tex->ID){return;}
	glBindTexture(GL_TEXTURE_2D, tex->ID);
	glUniform1i(tex->ID ,(GLint) 0);
	boundTexture = tex->ID;
}

u32 darken(u32 in){
	u32 b = (in >> 16) & 0xFF;
	u32 g = (in >>  8) & 0xFF;
	u32 r = (in      ) & 0xFF;

	r = (((r << 8) / 3) >> 7) & 0xFF;
	g = (((g << 8) / 3) >> 7) & 0xFF;
	b = (((b << 8) / 3) >> 7) & 0xFF;

	return (b<<16) | (g<<8) | (r) | 0xFF000000;
}

u32 interpol(u32 c1,u32 c2,u32 c3,u32 c4){
	u32 b = (((c1>>16)&0xFF) + ((c2>>16)&0xFF) + ((c3>>16)&0xFF) + ((c4>>16)&0xFF)) >> 2;
	u32 g = (((c1>> 8)&0xFF) + ((c2>> 8)&0xFF) + ((c3>> 8)&0xFF) + ((c4>> 8)&0xFF)) >> 2;
	u32 r = (((c1    )&0xFF) + ((c2    )&0xFF) + ((c3    )&0xFF) + ((c4    )&0xFF)) >> 2;

	return (b<<16) | (g<<8) | (r) | 0xFF000000;
}


void textureBuildBlockIcons(int loadFromFile){
	u32 *tblocks, *tgui;
	uint bw,bh,iw,ih;

	size_t blocks_len,gui_len;
	void *blocks_data,*gui_data;

	if(loadFromFile){
		blocks_data = loadFile(tBlocks->filename,&blocks_len);
		gui_data    = loadFile(tGui->filename,&gui_len);
	}else{
		blocks_data = gfx_blocks_png_data;
		blocks_len  = gfx_blocks_png_len;
		gui_data    = gfx_gui_png_data;
		gui_len     = gfx_gui_png_len;
	}

	lodepng_decode32((unsigned char **)&tblocks, &bw, &bh, blocks_data, blocks_len);
	lodepng_decode32((unsigned char **)&tgui, &iw, &ih, gui_data, gui_len);

	if(loadFromFile){
		free(blocks_data);
		free(gui_data);
	}

	for(uint i=0;i<256;i++){
		if(!blockTypeValid(i)){continue;}
		const int dx = (i & 0x1F) << 5;
		const int dy = (i >> 5)   << 5;
		int sx,sy;
		for(int y=0;y<32;y++){
			for(int x=0;x<32;x++){
				tgui[((y+dy)*iw) | (x+dx)] = 0x000000FF;
			}
		}
		sx = blockTypeGetTexX(i,0) << 5;
		sy = blockTypeGetTexY(i,0) << 5;
		for(int y=0;y<16;y++){
			for(int x=0;x<16;x++){
				const u32 c = interpol(
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)+1)],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)+1)]
				);
				tgui[((y+dy+8+(x>>1))*iw) | (x+dx)] = darken(c);
			}
		}
		sx = blockTypeGetTexX(i,4) << 5;
		sy = blockTypeGetTexY(i,4) << 5;
		for(int y=0;y<16;y++){
			for(int x=0;x<16;x++){
				const u32 c = interpol(
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)  )<<9) | (sx+(x<<1)+1)],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)  )],
					tblocks[((sy+(y<<1)+1)<<9) | (sx+(x<<1)+1)]
				);
				tgui[((y+dy+8+(7-(x>>1)))*iw) | (x+dx+16)] = darken(c);
			}
		}
		sx = blockTypeGetTexX(i,2) << 5;
		sy = blockTypeGetTexY(i,2) << 5;
		for(int y=0;y<32;y++){
			for(int x=0;x<32;x++){
				int ndx = 16 + (x>>1) - (y >> 1);
				int ndy = (x>>2) + (y>>2) + (x&1);
				tgui[((ndy+dy)*iw) | (ndx+dx)] = tblocks[((sy+y)<<9) | (sx+x)];
			}
		}
	}
	textureLoadSurface(tGui,iw,ih,tgui);
	free(tblocks);
	free(tgui);
}

void textureInit(){
	tBlocks      = textureNew(gfx_blocks_png_data,    gfx_blocks_png_len   ,"client/gfx/blocks.png");
	tCursor      = textureNew(gfx_cursor_png_data,    gfx_cursor_png_len   ,"client/gfx/cursor.png");
	tGui         = textureNew(gfx_gui_png_data,       gfx_gui_png_len      ,"client/gfx/gui.png");
	tCrosshair   = textureNew(gfx_crosshair_png_data, gfx_crosshair_png_len,"client/gfx/crosshair.png");
	tRope        = textureNew(gfx_rope_png_data,      gfx_rope_png_len     ,"client/gfx/rope.png");
	tBlockMining = textureNew(gfx_mining_png_data,    gfx_mining_png_len   ,"client/gfx/mining.png");
}

void reloadTexture(texture *tex){
	size_t dataLen;
	void *data;
	if((tex->filename == NULL) || (tex->filename[0] == 0)){return;}
	data = loadFile(tex->filename,&dataLen);
	if(data != NULL){
		fprintf(stderr,"Loading %s\n",tex->filename);
		textureLoad(tex,data,dataLen);
		free(data);
		if((tex == tBlocks) || (tex == tGui)){
			textureBuildBlockIcons(1);
		}
	}
}

void checkIfTextureNeedsReloading(texture *tex){
	struct stat statbuf;
	if(stat(tex->filename,&statbuf)){return;}
	if(statbuf.st_mtime <= tex->modTime){return;}
	tex->modTime = statbuf.st_mtime;
	reloadTexture(tex);
}

void checkTexturesForReloading(){
	static uint i=0;
	if(!optionRuntimeReloading){return;}
	if(++i >= textureCount){i=0;}
	checkIfTextureNeedsReloading(&textureList[i]);
}

void textureReload(){
	fprintf(stderr,"Reloading all textures\n");
	for(uint i=0;i<textureCount;i++){
		reloadTexture(&textureList[i]);
	}
}
