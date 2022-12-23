#include "../engine.h"
#include "entities.h"

static BOX_Chunk* target=NULL;
BOX_Chunk roundTree={
	589824,
/*Bottom Layer*/
	{
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
		{0,0,112,113,114,0,0,0,0,0,0,0,0,0,0,0,},
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
/*Top Layer*/
	{
		{0,0,52,53,54,0,0,0,0,0,0,0,0,0,0,0,},
		{0,61,62,63,64,65,0,0,0,0,0,0,0,0,0,0,},
		{70,71,72,73,74,75,0,0,0,0,0,0,0,0,0,0,},
		{80,81,82,83,84,85,86,0,0,0,0,0,0,0,0,0,},
		{90,91,92,93,94,95,96,0,0,0,0,0,0,0,0,0,},
		{100,101,102,103,104,105,106,0,0,0,0,0,0,0,0,0,},
		{0,111,0,0,0,0,0,0,0,0,0,0,0,0,0,0,},
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
		{8,0,},
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
	1,//Flags
/*Entity Spawns*/
	{
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
		{-1,"test arg",0,0},
	}
};

static void postbox(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	int ex=(receiver->x%(CHUNKSIZE*TILESIZE))/TILESIZE;
	int ey=(receiver->y%(CHUNKSIZE*TILESIZE))/TILESIZE;
	
	if(signal!=BOX_SIGNAL_SPAWN) {
		BOX_RemoveEntity(receiver->id);
		return;
	}
	
	if(CHUNKSIZE-ex<7 || CHUNKSIZE-ey<7)
			return;
	
	for(int y=0;y<8;y++) {
		for(int x=0;x<8;x++) {
			if(roundTree.bottom[y][x])
				target->bottom[y+ey][x+ex]=roundTree.bottom[y][x];
			if(roundTree.top[y][x])
				target->top[y+ey][x+ex]=roundTree.top[y][x];
			if(roundTree.clipping[y][x/8]&(1<<(x%8)))
				BOX_SetCollision(target, x+ex, y+ey, 1);
		}
	}
	//BOX_RemoveEntity(receiver->id);
}

BOX_Entity* ent_roundtree(int sx,int sy,const char* args,BOX_Chunk* chunk) {
	BOX_Entity* me=NEW(BOX_Entity);
	target=chunk;
		
	*me=(BOX_Entity){0};
	me->thumbnail=-1;
	me->postbox=postbox;
	me->tooltip="Tree";

	return me;
}
