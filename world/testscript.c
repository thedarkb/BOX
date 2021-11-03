#include "../engine.h"
#include "world.h"

#define TREEDENSITY 0xFFFFFFFF

void testscript(BOX_Chunk* self, int(*spawn)(BOX_Entity*,int,int,int,int)) {
	int hash=BOX_RandHash(self->id);
	for(int y=0;y<CHUNKSIZE;y++)
		for(int x=0;x<CHUNKSIZE/8;x++)
			self->clipping[y][x]=0;

	for(int i=0;i<TREEDENSITY;i++) {
		hash=BOX_RandHash(hash);
		int x=(hash&0xFF00)>>8;
		int y=(hash&0x00FF);
		spawn(ent_furniture(28),(x%CHUNKSIZE)*TILESIZE,(y%CHUNKSIZE)*TILESIZE,WORLD_GETX(self->id),WORLD_GETY(self->id));
		BOX_SetCollision(self,(x%CHUNKSIZE),y%CHUNKSIZE,1);
	}
}