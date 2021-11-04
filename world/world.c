#include "../engine.h"
#include "world.h"

#define BIT(x) (1<<x)

BOX_Chunk genCache[3][3];

void setCollision(unsigned int sX, unsigned int sY, char bit) {

}

static void treeGen(BOX_Chunk* self, int(*spawner)(BOX_Entity*,int,int,int,int)) {
	int hash=BOX_RandHash(self->id);

	for(int y=0;y<CHUNKSIZE;y++) {
		for(int x=0;x<CHUNKSIZE;x++) {
			if(hash<2000) {
				spawner(ent_furniture(1),x*TILESIZE,y*TILESIZE,self->id%65535,self->id/65535);
				BOX_SetCollision(self,x,y,1);
			}
			hash=BOX_RandHash(hash);
		}	
	}
}

BOX_Chunk* world_gen(unsigned int sX, unsigned int sY) {
	int cacheX=sX%3;
	int cacheY=sY%3;
	memset(genCache[cacheX][cacheY].bottom,20,sizeof genCache[cacheX][cacheY].bottom);
	memset(genCache[cacheX][cacheY].top,0,sizeof genCache[cacheX][cacheY].top);
	memset(genCache[cacheX][cacheY].clipping,0,sizeof genCache[cacheX][cacheY].clipping);
	genCache[cacheX][cacheY].initialiser=treeGen;
	genCache[cacheX][cacheY].id=WORLD_CHUNKID(sX,sY);

	return &genCache[cacheX][cacheY];
}