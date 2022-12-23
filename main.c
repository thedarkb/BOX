#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define EDENTS

#include "engine.h"
#include "entities/entities.h"
#include "world/world.h"

typedef struct _BOX_SpriteNode {
	int x,y,id,width,height;
	SDL_Texture* sheet;
	struct _BOX_SpriteNode* next;
} BOX_SpriteNode;

typedef struct _BOX_EntitySpawnItem {
	int sx,sy;
	const char* arg;
} BOX_EntitySpawnItem;

typedef struct _BOX_EventQueueNode {
	BOX_Signal signal;
	int target,sender;
	struct _BOX_EventQueueNode* next;
} BOX_EventQueueNode;

#define CAMERAX (BOX_CameraGet().x-RESX/2+TILESIZE/2)
#define CAMERAY (BOX_CameraGet().y-RESY/2+TILESIZE/2)

SDL_Window* w;
SDL_Surface* s;
SDL_Renderer* r;
SDL_Event keyIn;
SDL_Texture* sheet;
const uint8_t* k;

BOX_SignalHandler* signalHandlers=NULL;
BOX_EventQueueNode* messageBuffer=NULL;
BOX_Entity* entSet[ELIMIT]={NULL};
BOX_Chunk* chunkCache[3][3]={NULL};

unsigned int seed=42;
unsigned int rngstate;

unsigned int frameCounter=0;
unsigned int tileAnimClock=0;
BOX_entId nextId=0;
BOX_entId camera=0;

int sX=0x1;
int sY=0x1;

int entityTally=0;
int hashAttemptTally=0;
int hashMissTally=0;

BOX_SpriteNode* spriteList=NULL;
BOX_SpriteNode* spriteListEnd=NULL;

BOX_CameraPoint BOX_CameraGet() {
	static int frame=999;
	BOX_Entity* target=BOX_GetEntity(camera);
	if(target) {
		frame=frameCounter;
		return (BOX_CameraPoint){target->x,target->y};
	} else {
		frame=frameCounter;
		BOX_wprintf("The camera entity is dead or non-existent!\n");
		return (BOX_CameraPoint){CHUNKSIZE*TILESIZE,CHUNKSIZE*TILESIZE};
	}
}

uint16_t BOX_Rand() {
	rngstate ^= rngstate >> 7;
	rngstate ^= rngstate << 9;
	rngstate ^= rngstate >> 13;
	return rngstate;
}

void BOX_SetSeed(uint32_t in) {
	seed=in;
}

uint16_t BOX_RandHash(int n) {
	uint32_t state=seed^n;
	state|=n<<16;
	return (3*state%65357);
}

unsigned int BOX_Diff (int val1, int val2) {
	if (val1>val2) return val1-val2;
	else return val2-val1;
	return 0;
}

int BOX_SendMessage(int sender, int target, BOX_Signal type, BOX_Message in) {
	BOX_Entity *tP, *sP;
	if((tP=BOX_GetEntity(target)) && (sP=BOX_GetEntity(sender))) 
		tP->postbox(type,sP,tP,in);
	else
		return -1;
	
	return 0;
}

void BOX_ResolveEntityCollisions() {
	if(signalHandlers) {
	BOX_SignalHandler* walk=signalHandlers;
	do {
		BOX_Entity* collider;
		if(collider=BOX_GetEntity(walk->key)) {
			BOX_SignalHandler* subWalk=signalHandlers;
			do {
				BOX_Entity* target;
				if(target=BOX_GetEntity(subWalk->key)) {
					char xflag=0;
					char yflag=0;
					if(target==collider)
						continue;
					
					if(collider->x<target->x+target->bX && collider->x>target->x)
						xflag=1;
					if(collider->y<target->y+target->bY && collider->y>target->y)
						yflag=1;
				}
			} while(subWalk=subWalk->next);	
		}			
	} while(walk=walk->next);
	} else {
		BOX_panic("Nothing to do!\n");
	}	
}

char BOX_CollisionCheck(BOX_Entity* in,int x, int y) { //Collision detection between map layer and entity.
	//chunkCache[x/CHUNKSIZE-cameraXf/CHUNKSIZE/TILESIZE+1][y/CHUNKSIZE-cameraYf/CHUNKSIZE/TILESIZE+1]
	#ifdef EDITOR
	if(k[SDL_SCANCODE_LSHIFT])
		return 0;
	#endif
	
	for(int yOff=0;yOff<=in->bY;yOff+=in->bY/2) {
		for(int xOff=0;xOff<=in->bX;xOff+=in->bX/2) {
			int subX=((in->x%(CHUNKSIZE*TILESIZE))+xOff+x);
			int subY=((in->y%(CHUNKSIZE*TILESIZE))+yOff+y);
			int soffX=1;
			int soffY=1;
			
			if(subX<0) {
				soffX=0;
				subX+=CHUNKSIZE*TILESIZE;
			}
			if(subX>CHUNKSIZE*TILESIZE) {
				soffX=2;
				subX-=CHUNKSIZE*TILESIZE;
			}
			if(subY<0) {
				soffY=0;
				subY+=CHUNKSIZE*TILESIZE;
			}
			if(subY>CHUNKSIZE*TILESIZE) {
				//BOX_panic("Actually works.\n");
				soffY=2;
				subY-=CHUNKSIZE*TILESIZE;
			}
			subX/=TILESIZE;
			subY/=TILESIZE;
			
			if(subX==CHUNKSIZE || subY==CHUNKSIZE)
				continue;
			
			if(chunkCache[soffX][soffY]->clipping[subY][subX/8]&(1<<(subX%8))) {
				return 1;
			}
		}
	}
	return 0;
}

void BOX_SetCollision(BOX_Chunk* chunk, char x, char y, char z) {
	if(z)
		chunk->clipping[y][x/8]|=(1<<(x%8));
	else
		chunk->clipping[y][x/8]&=~(1<<(x%8));
}

char BOX_GetKey(int codeIn) {
	switch(codeIn) {
		case BOX_UP:
			return k[SDL_SCANCODE_UP];
		case BOX_DOWN:
			return k[SDL_SCANCODE_DOWN];
		case BOX_LEFT:
			return k[SDL_SCANCODE_LEFT];
		case BOX_RIGHT:
			return k[SDL_SCANCODE_RIGHT];
	}
}

void BOX_SetCamera(BOX_entId in) {
	camera=in;
}

BOX_entId BOX_NewEntityID() {
	return nextId;
}

uint16_t BOX_Hash(char *s) {//Lifted unashamedly from K&R2.
	unsigned int hashval;

	for (hashval = 0; *s != '\0'; s++)
		hashval = *s + 31*hashval;
	return hashval;
}

unsigned int BOX_FrameCount() {
	return frameCounter;
}

BOX_SignalHandler* BOX_RegisterHandler(BOX_entId owner, void(*handler)(BOX_Signal signal,BOX_Entity*,BOX_Entity*,BOX_Message)) {
	BOX_SignalHandler* newHead=malloc(sizeof(BOX_SignalHandler));
	newHead->item=handler;
	newHead->key=owner;
	newHead->next=signalHandlers;
	signalHandlers=newHead;
	return newHead;
}

int BOX_RemoveEntity(int id) {
	int index=id%ELIMIT;
	hashAttemptTally++;

	while(index<ELIMIT*3) {
		if(entSet[index%ELIMIT] && entSet[index%ELIMIT]->id==id) {
			if(entSet[index%ELIMIT]->state)
				free(entSet[index%ELIMIT]->state);
			free(entSet[index%ELIMIT]);
			entSet[index%ELIMIT]=NULL;
			entityTally--;
			return id;
		}
		index++;
		hashMissTally++;
	}
	BOX_eprintf("An attempt was made to remove a non-existent entity, id: %d.");
	return -1;
}

BOX_Entity* BOX_GetEntity(int id) {
	int index=id%ELIMIT;
	hashAttemptTally++;
	while(1) {
		if(entSet[index%ELIMIT] && entSet[index%ELIMIT]->id==id)
			return entSet[index%ELIMIT];
		if(index>ELIMIT*2)
			return NULL;
		index++;
		hashMissTally++;
	}
}

/*Returns first entity in table with specified tag. Returns subsequent entities if called with NULL.*/
BOX_Entity* BOX_GetEntityByTag(const char* tag) {
	static int lastId;
	static const char* lastTag;
	int i;
	
	if(tag) {
		lastTag=tag;
		lastId=-1;
	} else {
		tag=lastTag;
		if(!lastTag)
			return NULL;
	}
	for(i=lastId+1;i<ELIMIT;i++) {
		if(entSet[i])
			if(entSet[i]->tooltip)
				if(!strcmp(tag,entSet[i]->tooltip)){
					lastId=i;
					return entSet[i];
				}
	}
	return NULL;
}

int BOX_EntitySpawn(BOX_Entity* in,int x, int y) {
	int index=nextId%ELIMIT;
	if(!in) return -1;
	hashAttemptTally++;

	if(!in) {
		BOX_eprintf("Entity spawn called on NULL pointer.\n");
		return -1;
	}
	in->id=nextId;
	while(index<ELIMIT*2) {
		if(!entSet[index%ELIMIT]) {
			in->x=x;
			in->y=y;
			entSet[index%ELIMIT]=in;
			nextId++;
			entityTally++;

			if(in->postbox) {
				in->postbox(BOX_SIGNAL_SPAWN,NULL,in,MSG_EMPTY);//TODO: Replace NULL sender with Worldspawn.
				BOX_RegisterHandler(in->id,in->postbox);
			}			
			return in->id;
		}
		index++;
		hashMissTally++;
	}
	BOX_panic("Entity array full! Entity tally: %d, index wrapped to %d\n", entityTally,index);
	return -1;
}

int BOX_ChunkEntitySpawn(BOX_Entity* in, int x, int y, int sX, int sY) {
	return BOX_EntitySpawn(in,x+(sX*CHUNKSIZE*TILESIZE),y+(sY*CHUNKSIZE*TILESIZE));
}

void BOX_DrawBottom(int x, int y, int id) {
	if(!id) return;
	if(BOX_Diff(CAMERAX,x)>RESX) return;
	if(BOX_Diff(CAMERAY,y)>RESY) return;

	BOX_SpriteNode* new=NEW(BOX_SpriteNode);
	new->x=x;
	new->y=y;
	new->id=id;
	new->width=TILESIZE;
	new->height=TILESIZE;
	new->next=spriteList;
	spriteList=new;
	if(!spriteListEnd) spriteListEnd=spriteList;
}

void BOX_DrawTop(int x, int y, int id) {
	if(!id) return;
	if(BOX_Diff(CAMERAX,x)>RESX) return;
	if(BOX_Diff(CAMERAY,y)>RESY) return;
	BOX_SpriteNode* new=NEW(BOX_SpriteNode);
	new->x=x;
	new->y=y;
	new->id=id;
	new->width=TILESIZE;
	new->height=TILESIZE;
	new->next=NULL;
	if(spriteListEnd) spriteListEnd->next=new;
	else spriteList=new;
	spriteListEnd=new;
}

void BOX_RenderList() {
	int cX=CAMERAX;
	int cY=CAMERAY;
	int quadCount=0;
	BOX_SpriteNode* walk=spriteList;
	if(!spriteList) return;
	do {
		BOX_SpriteNode* old=walk;
		SDL_Rect source={(walk->id % (SPRITESHEET/walk->width))*walk->width, (walk->id / (SPRITESHEET/walk->width))*walk->height,walk->width,walk->height};
		SDL_Rect dest={walk->x-cX,walk->y-cY,walk->width,walk->height};

		SDL_RenderCopy(r,sheet,&source,&dest);
		quadCount++;
		
		walk=walk->next;
		free(old);
	} while(walk);
	spriteListEnd=spriteList=walk;
	if(k[SDL_SCANCODE_SPACE])
		printf("Number of quads drawn: %d\n",quadCount);
}

void chunkRefresh() {
	int regen=0;
	
	if(BOX_Diff(sX,CAMERASX)>0 && BOX_Diff(sX,CAMERASX)<2) {//Finds which chunks need regenerating.
		if(sX<CAMERASX) {
			regen=1;
			for(int x=0;x<2;x++) {
				for(int y=0;y<3;y++) {
					chunkCache[x][y]=chunkCache[x+1][y];
				}
			}
			for(int y=-1;y<2;y++) {
				BOX_EntitySpawn(ent_worldspawn(&chunkCache[2][y+1],CAMERASX+1,CAMERASY+y),0,0);
			}
			sX=CAMERASX;
			sY=CAMERASY;
		} else if(sX>CAMERASX) {
			regen=1;
			for(int x=2;x>0;x--) {
				for(int y=0;y<3;y++) {
					chunkCache[x][y]=chunkCache[x-1][y];
				}
			}
			for(int y=-1;y<2;y++) {
				BOX_EntitySpawn(ent_worldspawn(&chunkCache[0][y+1],CAMERASX-1,CAMERASY+y),0,0);
			}
			sX=CAMERASX;
			sY=CAMERASY;
		}
	} else if(BOX_Diff(sY,CAMERASY)>0 && BOX_Diff(sY,CAMERASY)<2) {
		if(sY<CAMERASY) {
			regen=1;
			for(int y=0;y<2;y++) {
				for(int x=0;x<3;x++) {
					chunkCache[x][y]=chunkCache[x][y+1];
				}
			}
			for(int x=-1;x<2;x++) {
				BOX_EntitySpawn(ent_worldspawn(&chunkCache[x+1][2],CAMERASX+x,CAMERASY+1),0,0);
			}
			sX=CAMERASX;
			sY=CAMERASY;
		} else if(sY>CAMERASY) {
			regen=1;
			for(int y=2;y>0;y--) {
				for(int x=0;x<3;x++) {
					chunkCache[x][y]=chunkCache[x][y-1];
				}
			}
			for(int x=-1;x<2;x++) {
				BOX_EntitySpawn(ent_worldspawn(&chunkCache[x+1][0],CAMERASX+x,CAMERASY-1),0,0);
			}
			sX=CAMERASX;
			sY=CAMERASY;
		}
	} else if(BOX_Diff(sX,CAMERASX)>1 || BOX_Diff(sY,CAMERASY)>1){
		regen=1;
		sX=CAMERASX;
		sY=CAMERASY;
		for(int x=0;x<3;x++)
			for(int y=0;y<3;y++)
				BOX_EntitySpawn(ent_worldspawn(&chunkCache[x][y],CAMERASX-1+x,CAMERASY-1+y),0,0);
	}
}

int 
loop() {
	static unsigned int lastTick=0;

	if(!frameCounter) {
		
		#ifdef EDITOR
		BOX_EntitySpawn(ent_editor(),241,161);
		#else
		BOX_EntitySpawn(ent_player(77),0,128);
		#endif
	}

	int delay=1000/FRAMERATE-(SDL_GetTicks()-lastTick);
	if(delay>0 && lastTick) SDL_Delay(delay);
	#ifndef EDITOR
	//else if(lastTick)
	//	BOX_wprintf("Game loop too slow by %d ticks!\n",SDL_GetTicks()-lastTick-1000/FRAMERATE);
	#endif
	lastTick=SDL_GetTicks();
	
	BOX_ResolveEntityCollisions();
	if(signalHandlers) {
		BOX_SignalHandler* walk=signalHandlers;
		do {
			if(walk->next) {
				if(!BOX_GetEntity(walk->next->key)) {
						BOX_SignalHandler* corpse=walk->next;
						walk->next=walk->next->next;
						free(corpse);
				}
			}
			if(BOX_GetEntity(walk->key)) walk->item(BOX_SIGNAL_FRAME,NULL,BOX_GetEntity(walk->key),MSG_FRAME(frameCounter));//TODO: Replace NULL sender with Worldspawn.
		} while(walk=walk->next);
	} else {
		BOX_panic("Nothing to do!\n");
	}
	if(k[SDL_SCANCODE_SPACE])
		printf("Ticks total after handling entities: %d\n",SDL_GetTicks()-lastTick);

	chunkRefresh();
	
	if(k[SDL_SCANCODE_SPACE])
		printf("Ticks total after chunk caching: %d\n",SDL_GetTicks()-lastTick);
	
	int cameraYf=BOX_CameraGet().y;
	int cameraXf=BOX_CameraGet().x;
	for(int y=(cameraYf-(RESY/2))/TILESIZE-TILESIZE/2;y<TILESIZE+(cameraYf-RESY/2)/TILESIZE;y++) {
		for(int x=(cameraXf-(RESX/2))/TILESIZE-TILESIZE/2;x<TILESIZE+TILESIZE/2+(cameraXf-RESX/2)/TILESIZE;x++) {
			int botTile=chunkCache[x/CHUNKSIZE-cameraXf/CHUNKSIZE/TILESIZE+1][y/CHUNKSIZE-cameraYf/CHUNKSIZE/TILESIZE+1]->bottom[y%CHUNKSIZE][x%CHUNKSIZE];
			int topTile=chunkCache[x/CHUNKSIZE-cameraXf/CHUNKSIZE/TILESIZE+1][y/CHUNKSIZE-cameraYf/CHUNKSIZE/TILESIZE+1]->top[y%CHUNKSIZE][x%CHUNKSIZE];
			
			if(botTile&ANIMFLAG)
				botTile+=tileAnimClock;
			if(topTile&ANIMFLAG)
				topTile+=tileAnimClock;
			
			BOX_DrawBottom(x*TILESIZE,y*TILESIZE,botTile&TILEMASK);
			BOX_DrawTop(x*TILESIZE,y*TILESIZE,topTile&TILEMASK);
		}
	}
	if(k[SDL_SCANCODE_SPACE])
		printf("Ticks total after rendering map: %d\n",SDL_GetTicks()-lastTick);
	
	if(k[SDL_SCANCODE_C])
		BOX_CollisionCheck(BOX_GetEntity(1),0,0);


	BOX_RenderList();
	SDL_RenderPresent(r);
	if(k[SDL_SCANCODE_SPACE])
		printf("Ticks total after drawing: %d\n",SDL_GetTicks()-lastTick);
	//lastTick=SDL_GetTicks();
	if(k[SDL_SCANCODE_SPACE] && !hashMissTally) printf("Entities: %d\tNo hash misses.\n",entityTally);
	if(k[SDL_SCANCODE_SPACE] && hashMissTally) {
		printf("Entities: %d\tHash miss average:%d\n",entityTally,hashAttemptTally/hashMissTally);
	}
	if(k[SDL_SCANCODE_SPACE])
		SDL_Delay(2000);
		
	if(frameCounter%5==0) {
		if(tileAnimClock+1<4)
			tileAnimClock++;
		else
			tileAnimClock=0;
	}
	tileAnimClock=0;//Disable tile animations as atlas space is needed.
	frameCounter++;
	return 0;
}

int main() {
	rngstate=seed;
	SDL_Init(SDL_INIT_VIDEO);
	w=SDL_CreateWindow(TITLE,0,0,RESX*SCALE,RESY*SCALE,SDL_WINDOW_OPENGL);
	r=SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	printf(SDL_GetError());
	k=SDL_GetKeyboardState(NULL);

	SDL_Surface* loader=IMG_Load("sheet.png");
	sheet=SDL_CreateTextureFromSurface(r, loader);
	SDL_FreeSurface(loader);

	SDL_RenderSetScale(r,SCALE,SCALE);
	
	#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop(loop,0,1);
	#endif

	while(1){
		if(loop()) return 0;
		while(SDL_PollEvent(&keyIn)) {
			if(keyIn.type == SDL_QUIT) return 0;
		}
	}
	return 0;
}
