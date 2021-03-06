#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef TITLE
#define TITLE "BOX"
#endif

#define RESX 240
#define RESY 160
#define SCALE 4
#define FRAMERATE 50

#define CHUNKSIZE 32 //Chunks are square.

#define ELIMIT 512
#define HANDLERLIMIT 64

#define SPRITESHEET 160
#define TILESIZE 16

#define CAMERASX ((BOX_CameraGet().x/TILESIZE)/CHUNKSIZE)
#define CAMERASY ((BOX_CameraGet().y/TILESIZE)/CHUNKSIZE)

#define NEW(x) malloc(sizeof(x))

#define BOX_eprintf(...) { fprintf(stderr, "Error on frame %d in %s: ",BOX_FrameCount(),__FILE__); fprintf(stderr, __VA_ARGS__); }
#define BOX_panic(...) {fprintf(stderr, "Fatal error on frame %d in %s: ",BOX_FrameCount(),__FILE__); fprintf(stderr, __VA_ARGS__); exit(1);}

#ifndef DISABLEWARNINGS
#define BOX_wprintf(...) {fprintf(stderr, "Warning on frame %d from %s: ",BOX_FrameCount(),__FILE__); fprintf(stderr, __VA_ARGS__);}
#else
#define BOX_wprintf(...)
#endif

typedef enum {
	BOX_SIGNAL_SPAWN,
	BOX_SIGNAL_FRAME,
	BOX_SIGNAL_COLLISION,
	BOX_SIGNAL_DEATH,
	BOX_SIGNAL_DIALOGUE,
	BOX_SIGNAL_BOUNDS//Used to demarcate the length of the signal handler array.
} BOX_Signal;

typedef enum {
	BOX_UP,
	BOX_DOWN,
	BOX_LEFT,
	BOX_RIGHT,
	BOX_A,
	BOX_B,
	BOX_L,
	BOX_R,
	BOX_START,
	BOX_SELECT
} BOX_Control;

typedef int BOX_entId;

typedef struct _BOX_SignalHandler {
	int key;
	void (*item)(BOX_Signal signal,BOX_entId,BOX_entId,void*);
	struct _BOX_SignalHandler* next;
} BOX_SignalHandler;

typedef struct _BOX_Entity {
	BOX_entId id;
	unsigned int x,y,bX,bY,hp,armour,thumbnail,class;
	void* state;
} BOX_Entity;

typedef struct _BOX_Chunk {
	uint32_t id;
	char name[12];//Used in the level editor.
	uint8_t bottom[CHUNKSIZE][CHUNKSIZE];
	uint8_t top[CHUNKSIZE][CHUNKSIZE];
	uint8_t clipping[CHUNKSIZE][CHUNKSIZE/8];
	uint8_t flags;
	void (*initialiser)(struct _BOX_Chunk* self, int(*spawner)(BOX_Entity*,int,int,int,int));
} BOX_Chunk;

typedef struct _cameraPoint {
	int x,y;
} BOX_CameraPoint;

BOX_CameraPoint BOX_CameraGet();
uint16_t BOX_Rand();
void BOX_SetSeed(uint32_t in);
uint16_t BOX_RandHash(int n);
unsigned int BOX_Diff (int val1, int val2);
void BOX_SetCollision(BOX_Chunk* chunk, char x, char y, char z);
char BOX_GetKey(int codeIn);
void BOX_SetCamera(BOX_entId in);
BOX_entId BOX_NewEntityID();
unsigned int BOX_Hash(char *s);
unsigned int BOX_FrameCount();
BOX_SignalHandler* BOX_RegisterHandler(BOX_Signal list,BOX_entId owner, void(*handler)(BOX_Signal,BOX_entId,BOX_entId,void*));
int BOX_RemoveEntity(int id);
BOX_Entity* BOX_GetEntity(int id);
int BOX_EntitySpawn(BOX_Entity* in,int x,int y);
int BOX_ChunkEntitySpawn(BOX_Entity* in, int x, int y, int sX, int sY);
void BOX_DrawBottom(int x, int y, int id);
void BOX_DrawTop(int x, int y, int id);
void BOX_RenderList();