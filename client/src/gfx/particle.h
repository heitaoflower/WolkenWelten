#pragma once
#include "../../../common/src/common.h"

extern int particleCount;

void particleInit();
void newParticleS(float x,float y,float z, unsigned int nrgba,float power,int nttl);
void newParticle (float x,float y,float z,float vx,float vy,float vz,float vvx,float vvy,float vvz,float size,float vsize,unsigned int nrgba,int nttl);
void particleUpdate();
void particleDraw();
