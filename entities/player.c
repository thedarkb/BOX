#include "../engine.h"
#include "entities.h"

#define SELF BOX_GetEntity(receiver)//This might segfault if it returns null, but god help you if the game is in a state where that may happen.

static void frameHandler(BOX_Signal signal, BOX_entId sender, BOX_entId receiver,void* state) {
	BOX_DrawTop(SELF->x,SELF->y,SELF->thumbnail);	
	if(BOX_GetKey(BOX_UP))SELF->y-=4;
	if(BOX_GetKey(BOX_DOWN))SELF->y+=4;
	if(BOX_GetKey(BOX_LEFT))SELF->x-=4;
	if(BOX_GetKey(BOX_RIGHT))SELF->x+=4;
}

static void setup(BOX_Signal signal, BOX_entId sender, BOX_entId receiver,void* state) {
	BOX_EntitySpawn(ent_camera(receiver),SELF->x,SELF->y);
}

BOX_Entity* ent_player(char sprite) {
	BOX_Entity* me=NEW(BOX_Entity);
	*me=(BOX_Entity){0};
	
	BOX_RegisterHandler(BOX_SIGNAL_FRAME, BOX_NewEntityID(),frameHandler);
	BOX_RegisterHandler(BOX_SIGNAL_SPAWN, BOX_NewEntityID(),setup);
	printf("Player spawned with ID: %d\n",BOX_NewEntityID());

	me->thumbnail=sprite;
	me->x=2352;
	me->y=8539;
	return me;
}