#include "../engine.h"
#include "entities.h"

#ifdef EDITOR
#include <SDL2/SDL.h>
#endif

#define SELF receiver

extern const uint8_t* k;

static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,void* state) {
	BOX_DrawTop(SELF->x-2,SELF->y,SELF->thumbnail);	
	if(BOX_GetKey(BOX_UP) && !BOX_CollisionCheck(receiver,0,-4))SELF->y-=4;
	if(BOX_GetKey(BOX_DOWN) && !BOX_CollisionCheck(receiver,0,4))SELF->y+=4;
	if(BOX_GetKey(BOX_LEFT) && !BOX_CollisionCheck(receiver,-4,0))SELF->x-=4;
	if(BOX_GetKey(BOX_RIGHT) && !BOX_CollisionCheck(receiver,4,0))SELF->x+=4;
	
	#ifdef EDITOR
	if(!(BOX_FrameCount()%15)) {
		if(k[SDL_SCANCODE_W])SELF->y-=TILESIZE*CHUNKSIZE;
		if(k[SDL_SCANCODE_S])SELF->y+=TILESIZE*CHUNKSIZE;
		if(k[SDL_SCANCODE_A])SELF->x-=TILESIZE*CHUNKSIZE;
		if(k[SDL_SCANCODE_D])SELF->x+=TILESIZE*CHUNKSIZE;
	}
	#endif
}

static void setup(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,void* state) {
	BOX_EntitySpawn(ent_camera(receiver->id),SELF->x,SELF->y);
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
	me->bX=11;
	me->bY=12;
	return me;
}
