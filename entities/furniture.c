#include "../engine.h"
#include "entities.h"
#include "assert.h"

BOX_Entity template={
	-1,
	0,
	0,
	16,
	16,
	0,
	65535,
	0,
	0,
	NULL
};

static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,void* state) {	
	BOX_DrawBottom(receiver->x,receiver->y,receiver->thumbnail);
	if(BOX_Diff(CAMERASX,receiver->x/TILESIZE/CHUNKSIZE)>1|| BOX_Diff(CAMERASY,receiver->y/TILESIZE/CHUNKSIZE)>1) {
		BOX_RemoveEntity(receiver->id);
	}
}

BOX_Entity* ent_furniture(int sx,int sy,const char* args,BOX_Chunk* chunk) {
	BOX_Entity* me=NEW(BOX_Entity);
	BOX_RegisterHandler(BOX_SIGNAL_FRAME, BOX_NewEntityID(),frameHandler);
	*me=template;
	me->thumbnail=atoi(args);
	
	#ifdef EDITOR
	me->tooltip="Furniture";
	#endif

	return me;
}
