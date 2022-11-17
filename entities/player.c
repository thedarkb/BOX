#include "../engine.h"
#include "entities.h"

#ifdef EDITOR
#include <SDL2/SDL.h>
#endif

#define SELF receiver

extern const uint8_t* k;

enum {
	NOPE,UP,DOWN,LEFT,RIGHT
};

static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,void* state) {
	static int animationTimer;
	static int lastDirection;
	char isMoving=0;
	char frame=56;//Position of player sprite on spritesheet.
	
	if(BOX_GetKey(BOX_UP) && !BOX_CollisionCheck(receiver,0,-4)) {
		SELF->y-=2;
		isMoving=UP;
	}
	if(BOX_GetKey(BOX_DOWN) && !BOX_CollisionCheck(receiver,0,4)) {
		SELF->y+=2;
		isMoving=DOWN;
	}
	if(BOX_GetKey(BOX_LEFT) && !BOX_CollisionCheck(receiver,-4,0)) {
		SELF->x-=2;
		isMoving=LEFT;
	}
	if(BOX_GetKey(BOX_RIGHT) && !BOX_CollisionCheck(receiver,4,0)) {
		SELF->x+=2;
		isMoving=RIGHT;
	}
	if(isMoving)
		lastDirection=isMoving;
		
	if(isMoving) {
		switch(animationTimer%4) {
			case 0:
				frame+=10;
			break;
			case 2:
				frame+=20;
			break;
		}
		BOX_DrawTop(SELF->x-2,SELF->y,frame+lastDirection-1);
	} else { 
		BOX_DrawTop(SELF->x-2,SELF->y,frame+lastDirection-1);
	}
	
	if(SELF->x>CHUNKSIZE*TILESIZE*10+TILESIZE*CHUNKSIZE-1)
			SELF->x-=CHUNKSIZE*TILESIZE*5;
	
	
	if(*(int*)state%15==0)
		animationTimer++;
}

static void setup(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message* state) {
	SELF->x=CHUNKSIZE*TILESIZE*5;
	SELF->y=CHUNKSIZE*TILESIZE*5;
	BOX_EntitySpawn(ent_camera(receiver->id),SELF->x,SELF->y);
}

static void signalSwitchboard(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message* state) {
	switch(signal) {
		case BOX_SIGNAL_FRAME:
			frameHandler(signal,sender,receiver,state);
		break;
		case BOX_SIGNAL_SPAWN:
			setup(signal,sender,receiver,state);
		break;
		default:
		break;
	}		
}


BOX_Entity* ent_player(char sprite) {
	BOX_Entity* me=NEW(BOX_Entity);
	*me=(BOX_Entity){0};
	
	me->postbox=signalSwitchboard;
	printf("Player spawned with ID: %d\n",BOX_NewEntityID());

	me->thumbnail=sprite;
	me->tag="Player";
	me->x=CHUNKSIZE*TILESIZE*3;
	me->y=CHUNKSIZE*TILESIZE*3;
	me->bX=11;
	me->bY=12;
	me->tag="Player";
	return me;
}
