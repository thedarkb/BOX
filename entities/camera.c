#include "../engine.h"
#include "entities.h"


void ent_camera(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	static BOX_Entity* target;
	if(signal==BOX_SIGNAL_SPAWN) {
		if(!sender) {
			BOX_wprintf("Camera spawned with no parent.\n");
			BOX_RemoveEntity(receiver->id);
			return;
		}
		receiver->hp=sender->id;//Dirty, I know, but when will I need to track the camera's HP??????
	}
	if(signal==BOX_SIGNAL_FRAME) {
		BOX_SetCamera(receiver->id);
	}
	if(signal!=BOX_UPDATE_CAMERA)
		return;
	if(target=BOX_GetEntity(receiver->hp)) {
		receiver->x=target->x;
		receiver->y=target->y;
	}
}
