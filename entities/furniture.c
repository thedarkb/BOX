#include "../engine.h"
#include "entities.h"
#include "assert.h"

void ent_furniture(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	if(signal==BOX_SIGNAL_SPAWN) {
		if(!state.spawnargs) {
			BOX_wprintf("Unable to spawn article of furniture: no arguments given.\n");
			BOX_RemoveEntity(receiver->id);
			return;
		}
		receiver->thumbnail=atoi(state.spawnargs);
	}
	if(signal!=BOX_SIGNAL_FRAME)
		return;
	BOX_DrawBottom(receiver->x,receiver->y,receiver->thumbnail);
	if(BOX_Diff(CAMERASX,receiver->x/TILESIZE/CHUNKSIZE)>1|| BOX_Diff(CAMERASY,receiver->y/TILESIZE/CHUNKSIZE)>1) {
		BOX_RemoveEntity(receiver->id);
	}
}
