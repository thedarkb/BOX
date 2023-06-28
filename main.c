#include <SDL2/SDL_image.h>
#include <SDL2/SDL.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#define EDENTS

#include "engine.h"
#include "font.h"
#include "entities/entities.h"
#include "world/world.h"

typedef struct _BOX_BGlayers {
	int id;
	SDL_Texture* page;
	SDL_Texture* overlay;
} BOX_BGlayers;

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

#define CHUNKPX CHUNKSIZE*TILESIZE

#define TEXTSTRINGS 16

SDL_Window* w;
SDL_Surface* s;
SDL_Renderer* r;
SDL_Event keyIn;
SDL_Texture* sheet;
SDL_Texture* font;
const uint8_t* k;

BOX_SignalHandler* signalHandlers=NULL;
BOX_EventQueueNode* messageBuffer=NULL;
BOX_Entity* entSet[ELIMIT]={NULL};
BOX_Chunk* chunkCache[3][3]={NULL};
BOX_BGlayers bgPages[3][3];

struct {
	char mode;//0 for fixed, 1 for relative.
	int r,g,b;
	int x,y;
	char* text;
} TextMessages[TEXTSTRINGS];
char textStack=0;

unsigned int seed=42;
unsigned int rngstate;

unsigned int frameCounter=0;
unsigned int tileAnimClock=0;
BOX_entId nextId=0;
BOX_entId camera=0;

int sX=0x1;
int sY=0x1;
char globalRegen=0;

int entityTally=0;
int hashAttemptTally=0;
int hashMissTally=0;

BOX_SpriteNode* spriteList=NULL;
BOX_SpriteNode* spriteListEnd=NULL;

//Populate array of pointers to entity spawn functions.
#define SPAWNER_LIST_ITEM(y) y,
	void (*entity_spawners[])(BOX_Signal, BOX_Entity*, BOX_Entity*,BOX_Message)={
		SPAWNER_LIST		
	};
#undef SPAWNER_LIST_ITEM

#define SPAWNER_LIST_ITEM(y) #y,
	const char* entity_string_names[]={
		SPAWNER_LIST
	};
#undef SPAWNER_LIST_ITEM

void ent_dummy(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	BOX_RemoveEntity(receiver->id);
	return;
}

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

BOX_entId BOX_Camera() {
	return camera;
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

unsigned int BOX_GetTicks() {
	return SDL_GetTicks();
}

int BOX_SendMessage(int sender, int target, BOX_Message in) {
	BOX_Entity *tP;
	BOX_Entity *sP=BOX_GetEntity(sender);
	if(tP=BOX_GetEntity(target))
		entity_spawners[tP->type](in.type,sP,tP,in);
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
	
	//if(x>TILESIZE/2 || y>TILESIZE/2 || x<-(TILESIZE/2) || y<-(TILESIZE/2))
	//	return 1;
	
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

BOX_SignalHandler* BOX_RegisterHandler(BOX_entId owner, int handler) {
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
			if(!strcmp(tag,entity_string_names[entSet[i]->type])){
				lastId=i;
				return entSet[i];
			}
	}
	return NULL;
}

int BOX_EntitySpawn(int in, const char* args, int x, int y) {
	return BOX_ChildEntitySpawn(in, NULL, args, x, y);
}

int BOX_ChildEntitySpawn(int in, BOX_Entity* parent, const char* args, int x, int y) {
	int index=nextId%ELIMIT;
	if(!in) return -1;
	hashAttemptTally++;

	if(in<0 || in>=SPAWNER_LIST_COUNT) {
		BOX_eprintf("Entity spawn called on invalid entity type: %d.\n",in);
		return -1;
	}
	while(index<ELIMIT*2) {
		if(!entSet[index%ELIMIT]) {
			int rval=nextId;
			entSet[index%ELIMIT]=malloc(sizeof(BOX_Entity));
			entSet[index%ELIMIT]->id=rval;
			entSet[index%ELIMIT]->type=in;
			entSet[index%ELIMIT]->state=NULL;
			entSet[index%ELIMIT]->x=x;
			entSet[index%ELIMIT]->y=y;
			entityTally++;
			nextId++;

			BOX_RegisterHandler(rval,in);
			entity_spawners[in](
				BOX_SIGNAL_SPAWN,
				parent,
				entSet[index%ELIMIT],
				MSG_SPAWN(args,chunkCache[(x/CHUNKSIZE)%3][(y/CHUNKSIZE)%3])
			);
	
			return rval;
		}
		index++;
		hashMissTally++;
	}
	BOX_wprintf("Entity array full! Entity tally: %d, index wrapped to %d\n", entityTally,index);
	return -1;
}

int BOX_ChunkEntitySpawn(int in,const char* args, int x, int y, int sX, int sY) {
	return BOX_EntitySpawn(in,args,x+(sX*CHUNKSIZE*TILESIZE),y+(sY*CHUNKSIZE*TILESIZE));
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

void BOX_RenderBGPage(BOX_Chunk* in) {
	int x=in->id%65535;
	int y=in->id/65535;
	int cacheX=x%3;
	int cacheY=y%3;
	SDL_SetRenderTarget(r,bgPages[cacheX][cacheY].page);
	for(int y=0;y<CHUNKSIZE;y++) {
		for(int x=0;x<CHUNKSIZE;x++) {
			SDL_Rect source={(in->bottom[y][x] % (SPRITESHEET/TILESIZE))*TILESIZE, (in->bottom[y][x] / (SPRITESHEET/TILESIZE))*TILESIZE,TILESIZE,TILESIZE};
			SDL_Rect dest={x*TILESIZE,y*TILESIZE,TILESIZE,TILESIZE};
			SDL_RenderCopy(r,sheet,&source,&dest);
		}
	}
	SDL_SetRenderTarget(r,NULL);
	bgPages[cacheX][cacheY].id=in->id;
}


void BOX_DrawBG() {
	int cX=CAMERAX;
	int cY=CAMERAY;
	for(int i=0;i<3;i++) {
		for(int j=0;j<3;j++) {
			int x=bgPages[i][j].id%65535;
			int y=bgPages[i][j].id/65535;
			int edge=CHUNKSIZE*TILESIZE;
			x*=CHUNKSIZE*TILESIZE;
			y*=CHUNKSIZE*TILESIZE;
			
			SDL_Rect dest={x-cX,y-cY,edge,edge};
			SDL_RenderCopy(r,bgPages[i][j].page,NULL,&dest);
		}
	}
}

void BOX_Text(int x, int y, char* text, int r, int g, int b, int mode) {
	if(textStack>=TEXTSTRINGS)
		return;
	TextMessages[textStack].r=r;
	TextMessages[textStack].g=g;
	TextMessages[textStack].b=b;
	TextMessages[textStack].mode=mode;//Relative position if 1;
	TextMessages[textStack].text=text;
	TextMessages[textStack].x=x;
	TextMessages[textStack++].y=y;
	return;
}

void BOX_DrawColouredText(int x, int y, char* text, int r, int g, int b) {
	BOX_Text(x,y,text,r,g,b,0);
	return;
}

void BOX_DrawColouredRelativeText(int x, int y, char* text, int r, int g, int b) {
	BOX_Text(x,y,text,r,g,b,1);
}

void BOX_DrawText(int x, int y, char* text) {
	BOX_Text(x,y,text,255,255,255,0);
	return;
}

void BOX_DrawRelativeText(int x, int y, char* text) {
	BOX_Text(x,y,text,255,255,255,1);
}

void renderText() {
	for(int i=0;i<textStack;i++) {
		char* curStr=TextMessages[i].text;
		int offsetX=TextMessages[i].x;
		int offsetY=TextMessages[i].y;
		SDL_SetTextureColorMod(font,TextMessages[i].r, TextMessages[i].g, TextMessages[i].b);
		if(TextMessages[i].mode) {
			offsetX-=CAMERAX;
			offsetY-=CAMERAY;
		}
		do {
			int cell=*curStr;
			const SDL_Rect src={(cell-' '-1)*7,0,7,7};
			SDL_Rect dst={offsetX,offsetY,7,7};
			if(cell=='\n') {
				offsetX=TextMessages[i].x;
				offsetY+=8;
				continue;
			}
			if(cell=='\t') {
				offsetX+=7*4;
				continue;
			}	
			SDL_RenderCopy(r,font,&src,&dst);
			offsetX+=7;									
		} while(*curStr++);
	}
	textStack=0;
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
	
	if(globalRegen) {
		for(int x=0;x<3;x++) {
			for(int y=0;y<3;y++) {
				//BOX_EntitySpawn(ent_worldspawn(&chunkCache[x][y],CAMERASX-1+x,CAMERASY-1+y),0,0);
				BOX_SendMessage(-1,BOX_EntitySpawn(SPAWN_ent_worldspawn,"",0,0),
					MSG_POPULATE_CHUNK(&chunkCache[x][y],CAMERASX-1+x,CAMERASY-1+y)
				);
				
				BOX_RenderBGPage(chunkCache[x][y]);
			}
		}
		globalRegen=0;
		return;
	}
	
	if(BOX_Diff(sX,CAMERASX)>0 && BOX_Diff(sX,CAMERASX)<2) {//Finds which chunks need regenerating.
		if(sX<CAMERASX) {
			regen=1;
			for(int x=0;x<2;x++) {
				for(int y=0;y<3;y++) {
					chunkCache[x][y]=chunkCache[x+1][y];
				}
			}
			for(int y=-1;y<2;y++) {
				//BOX_EntitySpawn(ent_worldspawn(&chunkCache[2][y+1],CAMERASX+1,CAMERASY+y),0,0);
				BOX_SendMessage(-1,BOX_EntitySpawn(SPAWN_ent_worldspawn,"",0,0),
					MSG_POPULATE_CHUNK(&chunkCache[2][y+1],CAMERASX+1,CAMERASY+y)
				);
				BOX_RenderBGPage(chunkCache[2][y+1]);
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
				//BOX_EntitySpawn(ent_worldspawn(&chunkCache[0][y+1],CAMERASX-1,CAMERASY+y),0,0);
				BOX_SendMessage(-1,BOX_EntitySpawn(SPAWN_ent_worldspawn,"",0,0),
					MSG_POPULATE_CHUNK(&chunkCache[0][y+1],CAMERASX-1,CAMERASY+y)
				);
				BOX_RenderBGPage(chunkCache[0][y+1]);
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
				//BOX_EntitySpawn(ent_worldspawn(&chunkCache[x+1][2],CAMERASX+x,CAMERASY+1),0,0);
				BOX_SendMessage(-1,BOX_EntitySpawn(SPAWN_ent_worldspawn,"",0,0),
					MSG_POPULATE_CHUNK(&chunkCache[x+1][2],CAMERASX+x,CAMERASY+1)
				);
				BOX_RenderBGPage(chunkCache[x+1][2]);
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
				//BOX_EntitySpawn(ent_worldspawn(&chunkCache[x+1][0],CAMERASX+x,CAMERASY-1),0,0);
				BOX_SendMessage(-1,BOX_EntitySpawn(SPAWN_ent_worldspawn,"",0,0),
					MSG_POPULATE_CHUNK(&chunkCache[x+1][0],CAMERASX+x,CAMERASY-1)
				);
				BOX_RenderBGPage(chunkCache[x+1][0]);
			}
			sX=CAMERASX;
			sY=CAMERASY;
		}
	} else if(BOX_Diff(sX,CAMERASX)>1 || BOX_Diff(sY,CAMERASY)>1){
		regen=1;
		sX=CAMERASX;
		sY=CAMERASY;
		for(int x=0;x<3;x++) {
			for(int y=0;y<3;y++) {
				//BOX_EntitySpawn(ent_worldspawn(&chunkCache[x][y],CAMERASX-1+x,CAMERASY-1+y),0,0);
				BOX_SendMessage(-1,BOX_EntitySpawn(SPAWN_ent_worldspawn,"",0,0),
					MSG_POPULATE_CHUNK(&chunkCache[x][y],CAMERASX-1+x,CAMERASY-1+y)
				);
				BOX_RenderBGPage(chunkCache[x][y]);
			}
		}
	}
}

int 
loop() {
	static unsigned int lastTick=0;

	if(!frameCounter) {
		#ifdef EDITOR
		BOX_EntitySpawn(SPAWN_ent_editor,"77",241,161);
		#else
		BOX_EntitySpawn(SPAWN_ent_player,"77",0,128);
		#endif
	}

	int delay=1000/FRAMERATE-(SDL_GetTicks()-lastTick);
	if(delay>0 && lastTick) SDL_Delay(delay);
	else if(lastTick && SDL_GetTicks()-lastTick-1000/FRAMERATE > 2)
		BOX_wprintf("Game loop too slow by %d ticks!\n",SDL_GetTicks()-lastTick-1000/FRAMERATE);
	lastTick=SDL_GetTicks();
	
	BOX_ResolveEntityCollisions();
	if(signalHandlers) {
		BOX_SignalHandler* walk=signalHandlers;
		BOX_Entity* recv;
		do {
			if(walk->next) {
				if(!BOX_GetEntity(walk->next->key)) {
						BOX_SignalHandler* corpse=walk->next;
						walk->next=walk->next->next;
						free(corpse);
				}
			}
			//if(BOX_GetEntity(walk->key)) walk->item(BOX_SIGNAL_FRAME,NULL,BOX_GetEntity(walk->key),MSG_FRAME(frameCounter));//TODO: Replace NULL sender with Worldspawn.
			if(recv=BOX_GetEntity(walk->key)) 
				entity_spawners[recv->type](BOX_SIGNAL_FRAME,NULL,recv,MSG_FRAME(frameCounter));
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
	BOX_DrawBG();
	for(int y=(cameraYf-(RESY/2))/TILESIZE-TILESIZE/2;y<TILESIZE+(cameraYf-RESY/2)/TILESIZE;y++) {
		for(int x=(cameraXf-(RESX/2))/TILESIZE-TILESIZE/2;x<TILESIZE+TILESIZE/2+(cameraXf-RESX/2)/TILESIZE;x++) {
			int dsx=x/CHUNKSIZE-cameraXf/CHUNKSIZE/TILESIZE+1;
			int dsy=y/CHUNKSIZE-cameraYf/CHUNKSIZE/TILESIZE+1;
			int botTile=chunkCache[dsx][dsy]->bottom[y%CHUNKSIZE][x%CHUNKSIZE];
			int topTile=chunkCache[dsx][dsy]->top[y%CHUNKSIZE][x%CHUNKSIZE];
			
			if(botTile&ANIMFLAG)
				botTile+=tileAnimClock;
			if(topTile&ANIMFLAG)
				topTile+=tileAnimClock;
			
			BOX_DrawTop(x*TILESIZE,y*TILESIZE,topTile&TILEMASK);
		}
	}
	if(k[SDL_SCANCODE_SPACE])
		printf("Ticks total after rendering map: %d\n",SDL_GetTicks()-lastTick);
	
	if(k[SDL_SCANCODE_C])
		BOX_CollisionCheck(BOX_GetEntity(1),0,0);

	BOX_RenderList();
	BOX_DrawColouredText(0,0,"Static Text Test.",frameCounter&0xFF,rand()&0xFF,rand()&0xFF);
	BOX_DrawRelativeText(5*CHUNKPX,5*CHUNKPX,"Relative Text Test.");
	BOX_DrawText(5,7,(char*)&chunkCache[1][1]);
	renderText();
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
//	tileAnimClock=0;//Disable tile animations as atlas space is needed.
	frameCounter++;
	return 0;
}

int main() {
	uint32_t texFormat;
	SDL_Surface* fontL;
	
	rngstate=seed;
	SDL_Init(SDL_INIT_VIDEO);
	w=SDL_CreateWindow(TITLE,0,0,RESX*SCALE,RESY*SCALE,SDL_WINDOW_OPENGL);
	r=SDL_CreateRenderer(w,-1,SDL_RENDERER_ACCELERATED /*| SDL_RENDERER_PRESENTVSYNC*/);
	printf(SDL_GetError());
	k=SDL_GetKeyboardState(NULL);

	SDL_Surface* loader=IMG_Load("sheet.png");
	sheet=SDL_CreateTextureFromSurface(r, loader);
	SDL_FreeSurface(loader);
	
	fontL=IMG_Load_RW(SDL_RWFromMem((void*)_font_png, _font_len),1);
	SDL_SetColorKey(fontL,SDL_TRUE,0);
	font=SDL_CreateTextureFromSurface(r,fontL);
	SDL_FreeSurface(fontL);

	SDL_RenderSetScale(r,SCALE,SCALE);
	SDL_QueryTexture(sheet,&texFormat,NULL,NULL,NULL);
	for(int y=0;y<3;y++)
		for(int x=0;x<3;x++)
			bgPages[y][x].page=SDL_CreateTexture(r,texFormat,SDL_TEXTUREACCESS_TARGET,CHUNKSIZE*TILESIZE,CHUNKSIZE*TILESIZE);
	
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
