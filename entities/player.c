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
	static unsigned int lastTicks;
	unsigned int curTicks=BOX_GetTicks();
	
	movSpd=(movSpd*((curTicks-lastTicks)/(1000/FRAMERATE-1)));
	if(movSpd>TILESIZE/2) {//Clamp player speed when the game slows down to allow them to move through doorways and things more easily.
		movSpd=TILESIZE/2;
	}
	
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
			if(!BOX_CollisionCheck(receiver,0,-(movSpd+2)))
				SELF->y-=movSpd;
			isMoving=UP;
		}
		if(BOX_GetKey(BOX_DOWN)) {
			if(!BOX_CollisionCheck(receiver,0,(movSpd+2)))
				SELF->y+=movSpd;
				if(isMoving==UP)
					isMoving=0;
				else
					isMoving=DOWN;
		}
		if(BOX_GetKey(BOX_LEFT)) {
			if(!BOX_CollisionCheck(receiver,-(movSpd+2),0))
				SELF->x-=movSpd;
			isMoving=LEFT;
		}
		if(BOX_GetKey(BOX_RIGHT)) {
			if(!BOX_CollisionCheck(receiver,(movSpd+2),0))
				SELF->x+=movSpd;
				if(isMoving==LEFT)
					isMoving=0;
				else
					isMoving=RIGHT;
		}
	}
	if(isMoving)
		lastDirection=isMoving;
	//BOX_GetEntity(BOX_Camera())->postbox(BOX_UPDATE_CAMERA,receiver,BOX_GetEntity(BOX_Camera()),MSG_CAMERAUPDATE(SELF->x,SELF->y));
	BOX_SendMessage(receiver->id,BOX_Camera(),MSG_CAMERAUPDATE(SELF->x,SELF->y));
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
	lastTicks=curTicks;
}

static void setup(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	char idtextbuf[4];
	printf("Player spawned with ID: %d\n",receiver->id);
	sprintf(idtextbuf,"%d",receiver->id);

	SELF->thumbnail=atoi(state.spawnargs);
	SELF->x=CHUNKSIZE*TILESIZE*3;
	SELF->y=CHUNKSIZE*TILESIZE*3;
	SELF->bX=11;
	SELF->bY=12;
	SELF->x=CHUNKSIZE*TILESIZE*5;
	SELF->y=CHUNKSIZE*TILESIZE*5+30;
	BOX_ChildEntitySpawn(SPAWN_ent_camera,receiver,NULL,SELF->x,SELF->y);
}

void ent_player(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	switch(signal) {
		case BOX_SIGNAL_FRAME:
			frameHandler(signal,sender,receiver,state);
		break;
		case BOX_SIGNAL_SPAWN:
			setup(signal,sender,receiver,state);
		break;
		default:
		BOX_wprintf("Unknown event received: %d\n",signal);
		break;
	}		
}
