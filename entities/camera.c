#include "../engine.h"
#include "entities.h"


static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,void* state) {
	BOX_Entity* target;
	if(target=BOX_GetEntity(receiver->hp)) {
		receiver->x=target->x;
		receiver->y=target->y;
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
