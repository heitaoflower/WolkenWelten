/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "rain.h"

#include "../game/character.h"
#include "../game/weather.h"
#include "../gfx/gfx.h"
#include "../gfx/gl.h"
#include "../gfx/mat.h"
#include "../gfx/particle.h"
#include "../gfx/shader.h"

uint rainVAO;
uint rainVBO;
uint rainVBOSize = 0;

#include <math.h>

#ifdef __x86_64__
int rainFakeIters = 32;
#else
int rainFakeIters = 16;
#endif


void rainInitGfx(){
	glGenVertexArrays(1, &rainVAO);
	glGenBuffers     (1, &rainVBO);
	glBindVertexArray    (rainVAO);
	glEnableVertexAttribArray(0);
}

void rainFakeDrops(){
	if(rainIntensity == 0){return;}
	vec pos = player->pos;
	int cy = (((int)pos.y) & 0xFE00)-0x100;
	pos.y = cy + (32.f - 256.f);
	for(uint i=0;i<8;i++){
		float v = 48.f;
		for(int ii=0;ii<4;ii++){
			for(int iii=0;iii<rainFakeIters;iii++){
				if(rngValA(255) > rainIntensity){continue;}
				const vec rpos = vecAdd(pos,vecMul(vecRng(), vecNew( v,0.f, v)));
				const u8 vv = cloudTex[(uint)(rpos.x - cloudOff.x)&0xFF][(uint)(rpos.z - cloudOff.z)&0xFF];
				if(vv < cloudDensityMin){continue;}
				rainNew(rpos);
			}
			v *= 2.f;
		}
		pos.y += 256.f;
	}
}

void rainDrawAll(){
	if(!rainCount){return;}
	rainFakeDrops();

	shaderBind(sRain);
	matMul(matMVP,matView,matProjection);
	shaderMatrix(sParticle,matMVP);
	shaderSizeMul(sRain,1.f + (player->aimFade * player->zoomFactor));
	glDepthMask(GL_FALSE);

	glBindVertexArray(rainVAO);
	glBindBuffer(GL_ARRAY_BUFFER,rainVBO);
	if(gfxUseSubData && (rainVBOSize >= rainCount)){
		glBufferSubData(GL_ARRAY_BUFFER, 0, rainCount*sizeof(glRainDrop), glRainDrops);
	}else{
		glBufferData(GL_ARRAY_BUFFER, rainCount*sizeof(glRainDrop), glRainDrops, GL_DYNAMIC_DRAW);
		rainVBOSize = rainCount;
	}
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (void *)0);
	glDrawArrays(GL_POINTS,0,rainCount);

	glDepthMask(GL_TRUE);
}

void rainRecvUpdate(const packet *p){
	const vec pos  = vecNew(p->v.f[0],p->v.f[1],p->v.f[2]);
	const vec dist = vecSub(pos,player->pos);
	const float dd = vecDot(dist,dist);
	if(dd > renderDistanceSquare){return;}
	rainNew(pos);
}
