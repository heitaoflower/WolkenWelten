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

#include "../game/entity.h"

#include "../main.h"
#include "../sdl/sfx.h"
#include "../game/character.h"
#include "../gfx/frustum.h"
#include "../gfx/gfx.h"
#include "../gfx/mat.h"
#include "../gfx/mesh.h"
#include "../gfx/shader.h"
#include "../gfx/shadow.h"
#include "../gfx/sky.h"
#include "../gfx/texture.h"
#include "../tmp/assets.h"
#include "../tmp/objs.h"
#include "../voxel/bigchungus.h"

#include <stdlib.h>
#include <math.h>
#include "../gfx/gl.h"

#define ENTITY_FADEOUT (256.f)

void entityDraw(const entity *e){
	if(e->eMesh == NULL){return;}

	matMov      (matMVP,matView);
	matMulTrans (matMVP,e->pos.x,e->pos.y,e->pos.z);
	matMulRotYX (matMVP,e->rot.yaw,e->rot.pitch);
	matMulScale (matMVP,0.4f,0.4f,0.4f);
	matMul      (matMVP,matMVP,matProjection);

	shaderMatrix(sMesh,matMVP);
	meshDraw(e->eMesh);
	shadowAdd(e->pos,0.5f);
}

void entityDrawAll(){
	shaderBind(sMesh);
	shaderBrightness(sMesh,worldBrightness);
	for(uint i=0;i<entityCount;i++){
		if(entityList[i].nextFree != NULL)                        { continue; }
		if(entityDistance(&entityList[i],player) > ENTITY_FADEOUT){ continue; }
		if(!CubeInFrustum(vecSubS(entityList[i].pos,.5f),1.f))    { continue; }
		entityDraw(&entityList[i]);
	}
}
