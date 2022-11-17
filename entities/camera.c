#include "../engine.h"
#include "entities.h"


static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message* state) {
	BOX_Entity* target;
	if(signal!=BOX_SIGNAL_FRAME)
		return;
	if(target=BOX_GetEntity(receiver->hp)) {
		receiver->x=target->x;
		receiver->y=target->y;
	}
}

BOX_Entity* ent_camera(BOX_entId target) {
	BOX_Entity* me=NEW(BOX_Entity);
	memset(me,0,sizeof *me);
	me->postbox=frameHandler;
	BOX_SetCamera(BOX_NewEntityID());
	me->tag="Camera";

	me->hp=target;//Dirty, I know, but when will I need to track the camera's HP??????
	return me;
}
