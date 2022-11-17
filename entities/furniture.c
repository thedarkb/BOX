#include "../engine.h"
#include "entities.h"
#include "assert.h"

BOX_Entity template={
	-1,
	NULL,
	NULL,
	0,
	0,
	4,
	4,
	0,
	65535,
	0,
	0,
	NULL
};

static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message* state) {
	if(signal!=BOX_SIGNAL_FRAME)
		return;
	BOX_DrawBottom(receiver->x,receiver->y,receiver->thumbnail);
	if(BOX_Diff(CAMERASX,receiver->x/TILESIZE/CHUNKSIZE)>1|| BOX_Diff(CAMERASY,receiver->y/TILESIZE/CHUNKSIZE)>1) {
		BOX_RemoveEntity(receiver->id);
	}
}

BOX_Entity* ent_furniture(int sx,int sy,const char* args,BOX_Chunk* chunk) {
	BOX_Entity* me=NEW(BOX_Entity);
	
	*me=template;
	me->thumbnail=atoi(args);
	me->postbox=frameHandler;
	me->tag="Furniture";
	#ifdef EDITOR
	me->tooltip="Furniture";
	#endif

	return me;
}
