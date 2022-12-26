#include "world.h"
#include "chunks.h"

enum mapflags {
	WS_CHUNK,
	WS_MODIFIED,
};

#define BIT(x) (1<<x)

BOX_Chunk genCache[3][3];

void setCollision(unsigned int sX, unsigned int sY, char bit) {

}

static void treeGen(BOX_Chunk* self, int(*spawner)(BOX_Entity*,int,int,int,int)) {
	int hash=BOX_RandHash(self->id);
	memset(self,0,sizeof *self);
	memset(self->bottom,20,sizeof self->bottom);
	for(int i=0;i<127;i++) {
		hash=BOX_RandHash(hash);
		int x=(hash&0xFF00)>>8;
		int y=(hash&0x00FF);
		self->bottom[y%CHUNKSIZE][x%CHUNKSIZE]=1;
		BOX_SetCollision(self,(x%CHUNKSIZE),y%CHUNKSIZE,1);
	}
}

BOX_Entity* ent_worldspawn(BOX_Chunk** self,unsigned int sX, unsigned int sY) {
	int cacheX=sX%3;
	int cacheY=sY%3;
	char chunkfound=0;
	
	BOX_Chunk testChunk={
		WORLD_CHUNKID(sX,sY),
/*Bottom Layer*/
		{
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
			{2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,},
		},
/*Top Layer*/
		{
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
			{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		},
/*Clipping Layer*/
		{
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
			{0,0,},
		},
		3,//Flags
/*Entity Spawns*/
		{
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
			{-1,NULL,0,0},
		}
	};


	for(int i=0;i<worldArray_len;i++) {
		if(worldArray[i].id==WORLD_CHUNKID(sX,sY)) {
			worldArray[i].flags=BIT(WS_CHUNK);
			genCache[cacheX][cacheY]=worldArray[i];
			genCache[cacheX][cacheY].flags=BIT(WS_CHUNK);
			chunkfound=1;			
		}
	}
	
	if(!chunkfound)
		genCache[cacheX][cacheY]=testChunk;
		
	genCache[cacheX][cacheY].id=WORLD_CHUNKID(sX,sY);
	*self=&genCache[cacheX][cacheY];
	

	
	for(int i=0;i<CHUNK_ELIMIT;i++) {
		if(genCache[cacheX][cacheY].entities[i].entitySpawner>0) {
			if(genCache[cacheX][cacheY].entities[i].args) {
				BOX_ChunkEntitySpawn(
					editor_entities[genCache[cacheX][cacheY].entities[i].entitySpawner](
						sX*CHUNKSIZE+genCache[cacheX][cacheY].entities[i].x,
						sY*CHUNKSIZE+genCache[cacheX][cacheY].entities[i].y,
						genCache[cacheX][cacheY].entities[i].args,
						&genCache[cacheX][cacheY]
					),
					genCache[cacheX][cacheY].entities[i].x*TILESIZE,
					genCache[cacheX][cacheY].entities[i].y*TILESIZE,
					sX,
					sY
				);
			} else {
				BOX_ChunkEntitySpawn(
					editor_entities[genCache[cacheX][cacheY].entities[i].entitySpawner](
						sX*CHUNKSIZE+genCache[cacheX][cacheY].entities[i].x,
						sY*CHUNKSIZE+genCache[cacheX][cacheY].entities[i].y,
						NULL,
						&genCache[cacheX][cacheY]
					),
					genCache[cacheX][cacheY].entities[i].x*TILESIZE,
					genCache[cacheX][cacheY].entities[i].y*TILESIZE,
					sX,
					sY
				);
			}
		}
	}
	return NULL;
}
