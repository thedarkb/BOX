#include "../engine.h"
#include "entities.h"
#include "assert.h"

#define SELF BOX_GetEntity(receiver)

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

static void frameHandler(BOX_Signal signal, BOX_entId sender, BOX_entId receiver,void* state) {
	BOX_DrawBottom(SELF->x,SELF->y,SELF->thumbnail);
	if(BOX_Diff(CAMERASX,SELF->x/TILESIZE/CHUNKSIZE)>1|| BOX_Diff(CAMERASY,SELF->y/TILESIZE/CHUNKSIZE)>1) {
		BOX_RemoveEntity(receiver);
	}
}

BOX_Entity* ent_furniture(char sprite) {
	BOX_Entity* me=NEW(BOX_Entity);
	BOX_RegisterHandler(BOX_SIGNAL_FRAME, BOX_NewEntityID(),frameHandler);
	*me=template;
	me->thumbnail=sprite;

	return me;
}