#include "projectile.h"

#include "../game/character.h"
#include "../sdl/sfx.h"
#include "../gfx/particle.h"

static inline void projectileDrawFlameBullet(const projectile *p){
	for(int ii=0;ii<4;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 192, -6.f,0xFF3060B0,96);
	}
	for(int ii=0;ii<2;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -8.f,0xFF70B8FF,64);
	}
}

static inline void projectileDrawWaterBullet(const projectile *p){
	for(int ii=0;ii<4;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 192, -6.f,0xFFB05020,192);
	}
	for(int ii=0;ii<2;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -8.f,0xFF9F7830,128);
	}
}

static inline void projectileDrawAssaultBullet(const projectile *p){
	vec cp,cv;
	const u32 colors[] = {
		0x00040723,
		0x00051F39,
		0x000F0B2B,
		0x000C1D38
	};
	u32 cc = colors[(p - projectileList)&3];
	cp = p->pos;
	cv = vecMulS(p->vel,-0.2f);
	for(int ii=0;ii<10;ii++){
		cp = vecAdd(cp,cv);
		newParticleV(cp, vecMulS(vecRng(),0.001f), 156, -6.f,0xFF50A0F0 | cc,96);
	}
	cp = p->pos;
	cv = vecMulS(p->vel,-0.1f);
	for(int i=0;i<40;i++){
		cp = vecAdd(cp,cv);
		newParticleV(cp, vecZero(), 64, -.4f,0xFF2060C0 | cc,128);
	}

	for(int ii=0;ii<4;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 192, -6.f,0xFF50A0F0 | cc,96);
	}
	for(int ii=0;ii<2;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -8.f,0xFF70B8FF | cc,64);
	}
}

static inline void projectileDrawShotgunBullet(const projectile *p){
	vec cp,cv;
	u32 tc = 0xFF2060C0 | (rngValR() & 0x070F3F);
	cp = p->pos;
	cv = vecMulS(p->vel,-0.1f);
	for(int i=0;i<40;i++){
		cp = vecAdd(cp,cv);
		newParticleV(cp, vecZero(), 64, -.4f,tc,128);
	}
	for(int ii=0;ii<2;ii++){
		newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -6.f,0xFF50D0F0,78);
	}
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 384, -8.f,0xFF90E8FF,48);
}

static inline float projectileDrawGuardianProjectile(const projectile *p){
	newParticleV(p->pos, vecZero(), 192, -.1f,0xFF3070E0,1024);
	newParticleV(p->pos, vecMulS(vecRng(),0.01f), 256, -4.f,0xFF50A0F0,256);
	return vecMag(vecSub(p->pos,player->pos));
}

static inline float projectileDrawGuardianBigProjectile(const projectile *p){
	newParticleV(p->pos, vecZero(), 384, -.1f,0xFF205FE9,2048);
	newParticleV(p->pos, vecMulS(vecRng(),0.015f), 512, -4.f,0xFF4670FF,256);
	return vecMag(vecSub(p->pos,player->pos));
}

void projectileDrawAll(){
	float maxD = 4096.f;
	for(uint i=0;i<countof(projectileList);i++){
		projectile *p = &projectileList[i];
		switch(p->style){
		default:
		case 0:
			break;
		case 1:
			projectileDrawAssaultBullet(p);
			break;
		case 2: {
			const float d = projectileDrawGuardianProjectile(p);
			maxD = MIN(d,maxD);
			break; }
		case 3: {
			const float d = projectileDrawGuardianBigProjectile(p);
			maxD = MIN(d,maxD);
			break; }
		case 4:
			projectileDrawShotgunBullet(p);
			break;
		case 5:
			projectileDrawFlameBullet(p);
			break;
		case 6:
			projectileDrawWaterBullet(p);
			break;
		}
	}
	if(maxD < 128.f){
		sfxLoop(sfxProjectile,((128.f - maxD)/128.F)*0.5f);
	}else{
		sfxLoop(sfxProjectile,0.f);
	}
}
