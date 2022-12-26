#include "../engine.h"
#include "entities.h"

static void bush_box(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
}

BOX_Entity* ent_plant(int sx,int sy,const char* args,BOX_Chunk* chunk) {
	BOX_Entity* me=NEW(BOX_Entity);
		
	*me=(BOX_Entity){0};
	me->thumbnail=-1;
	me->postbox=bush_box;
	me->tooltip="Plant";

	return me;
}
