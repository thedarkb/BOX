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

static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	static int animationTimer;
	static int lastDirection=DOWN;
	static char isMoving=0;
	unsigned char frame=186;//Position of player sprite on spritesheet.
	unsigned char movSpd=2,keyCount=0;
	
	for(int i=1;i<5;i++) {
		if(BOX_GetKey(i))
			keyCount++;
	}
	
	#ifdef EDITOR
	if(k[SDL_SCANCODE_K])
		movSpd=255;
	#endif
	
	if(BOX_FrameCount()%5!=0) {
		isMoving=0;
		if(BOX_GetKey(BOX_UP)) {
			if(!BOX_CollisionCheck(receiver,0,-4))
				SELF->y-=movSpd;
			isMoving=UP;
		}
		if(BOX_GetKey(BOX_DOWN)) {
			if(!BOX_CollisionCheck(receiver,0,4))
				SELF->y+=movSpd;
				if(isMoving==UP)
					isMoving=0;
				else
					isMoving=DOWN;
		}
		if(BOX_GetKey(BOX_LEFT)) {
			if(!BOX_CollisionCheck(receiver,-4,0))
				SELF->x-=movSpd;
			isMoving=LEFT;
		}
		if(BOX_GetKey(BOX_RIGHT)) {
			if(!BOX_CollisionCheck(receiver,4,0))
				SELF->x+=movSpd;
				if(isMoving==LEFT)
					isMoving=0;
				else
					isMoving=RIGHT;
		}
	}
	if(isMoving)
		lastDirection=isMoving;
	BOX_GetEntity(BOX_Camera())->postbox(BOX_UPDATE_CAMERA,receiver,BOX_GetEntity(BOX_Camera()),MSG_CAMERAUPDATE(SELF->x,SELF->y));
	if(isMoving) {
		if(animationTimer%2==0)
			frame+=10;
		else
			frame+=20;
		BOX_DrawTop(SELF->x-2,SELF->y,frame+lastDirection-1);
	} else { 
		BOX_DrawTop(SELF->x-2,SELF->y,frame+lastDirection-1);
	}
	
	
	if(state.frame%15==0)
		animationTimer++;
}

static void setup(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	SELF->x=CHUNKSIZE*TILESIZE*5;
	SELF->y=CHUNKSIZE*TILESIZE*5+30;
	BOX_EntitySpawn(ent_camera(receiver->id),SELF->x,SELF->y);
}

static void signalSwitchboard(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	switch(signal) {
		case BOX_SIGNAL_FRAME:
			frameHandler(signal,sender,receiver,state);
		break;
		case BOX_SIGNAL_SPAWN:
			setup(signal,sender,receiver,state);
		break;
		default:
		BOX_wprintf("Unknown event received: %d",signal);
		break;
	}		
}


BOX_Entity* ent_player(char sprite) {
	BOX_Entity* me=NEW(BOX_Entity);
	*me=(BOX_Entity){0};
	
	me->postbox=signalSwitchboard;
	printf("Player spawned with ID: %d\n",BOX_NewEntityID());

	me->thumbnail=sprite;
	me->x=CHUNKSIZE*TILESIZE*3;
	me->y=CHUNKSIZE*TILESIZE*3;
	me->bX=11;
	me->bY=12;
	return me;
}
