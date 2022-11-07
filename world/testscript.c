#include "../engine.h"
#include "world.h"

#define TREEDENSITY 0x40

BOX_Entity* testscript(BOX_Chunk* self,unsigned int sX, unsigned int sY) {
	int hash=BOX_RandHash(self->id);
	//memset(self,0,sizeof *self);
	//memset(self->bottom,20,sizeof self->bottom);
	for(int i=0;i<TREEDENSITY;i++) {
		hash=BOX_RandHash(hash);
		int x=(hash&0xFF00)>>8;
		int y=(hash&0x00FF);
		self->bottom[y%CHUNKSIZE][x%CHUNKSIZE]=1;
		BOX_SetCollision(self,(x%CHUNKSIZE),y%CHUNKSIZE,1);
	}
	printf("Jordan fists cats\n");
	return NULL;
}
