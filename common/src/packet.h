#pragma once
#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
	uint32_t typesize;

	/*
	 *  type     = typesize & 0xFF
	 *  size     = typesize >> 10
	 */

	union {
		float    f[1];
		int32_t  i[1];
		uint32_t u[1];
		uint16_t s[2];
		uint8_t  c[4];
	} val;
} packet;

#pragma pack(pop)
inline unsigned int alignedLen(unsigned int size){
	if(size & 0x3){
		return (size & (~0x3))+4;
	}else{
		return (size & (~0x3));
	}
}
inline unsigned int packetLen(packet *p){
	return p->typesize >> 10;
}
inline unsigned int packetType(packet *p){
	return p->typesize & 0xFF;
}
inline void packetSet(packet *p, uint8_t ptype, uint32_t len){
	p->typesize = (uint32_t)ptype | (len << 10);
}

void packetQueue         (packet *p, uint8_t ptype, uint32_t len, int c);
void packetQueueExcept   (packet *p, uint8_t ptype, uint32_t len, int c);
void packetQueueToServer (packet *p, uint8_t ptype, uint32_t len);