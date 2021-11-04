#include "../engine.h"
#include "entities.h"

#define SELF BOX_GetEntity(receiver)

static void frameHandler(BOX_Signal signal, BOX_entId sender, BOX_entId receiver,void* state) {
	BOX_Entity* target;
	if(target=BOX_GetEntity(SELF->hp)) {
		SELF->x=target->x;
		SELF->y=target->y;
	}
}

BOX_Entity* ent_camera(BOX_entId target) {
	BOX_Entity* me=NEW(BOX_Entity);
	memset(me,sizeof *me,0);
	BOX_RegisterHandler(BOX_SIGNAL_FRAME, BOX_NewEntityID(),frameHandler);
	BOX_SetCamera(BOX_NewEntityID());

	me->hp=target;//Dirty, I know, but when will I need to track the camera's HP??????
	return me;
}