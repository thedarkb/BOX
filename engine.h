#ifndef ENGINE_H
#define ENGINE_H

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifndef TITLE
#define TITLE 			"BOX"
#endif

#define RESX 			240
#define RESY 			160
#define SCALE			4
#define FRAMERATE		60

#define CHUNKSIZE 		16 //Chunks are square.

#define ELIMIT 			512
#define CHUNK_ELIMIT 	32
#define HANDLERLIMIT	64

#define SPRITESHEET 	160
#define TILESIZE 		16

#define ANIMFLAG		0x80
#define TILEMASK		0x7F

#define CAMERASX 		((BOX_CameraGet().x/TILESIZE)/CHUNKSIZE)
#define CAMERASY 		((BOX_CameraGet().y/TILESIZE)/CHUNKSIZE)

#define NEW(x) 			malloc(sizeof(x))
#define ARRAY_LEN(x) (sizeof (x) / sizeof (x[0]))

#define BOX_eprintf(...) 	{fprintf(stderr, "Error on frame %d in %s on line %d: ",BOX_FrameCount(),__FILE__,__LINE__); fprintf(stderr, __VA_ARGS__); }
#define BOX_panic(...) 		{fprintf(stderr, "Fatal error on frame %d in %s on line %d: ",BOX_FrameCount(),__FILE__,__LINE__); fprintf(stderr, __VA_ARGS__); exit(1);}

#ifndef DISABLEWARNINGS
#define BOX_wprintf(...) 	{fprintf(stderr, "Warning on frame %d from %s on line %d: ",BOX_FrameCount(),__FILE__,__LINE__); fprintf(stderr, __VA_ARGS__);}
#else
#define BOX_wprintf(...)
#endif

#ifdef EDITOR
#define BOX_debugmsg(...) 	{fprintf(stderr, "Message on frame %d from %s on line %d: ",BOX_FrameCount(),__FILE__,__LINE__); fprintf(stderr, __VA_ARGS__);}
#else
#define BOX_debugmsg(...)
#endif

typedef enum {
	BOX_SIGNAL_SPAWN,
	BOX_SIGNAL_FRAME,
	BOX_SIGNAL_COLLISION,
	BOX_SIGNAL_DEATH,
	BOX_SIGNAL_DIALOGUE,
	BOX_UPDATE_CAMERA,
	BOX_POPULATE_CHUNK,//Used only by ent_worldspawn.
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

struct _BOX_Message;

typedef struct _BOX_Entity {
	BOX_entId id;
	int type;//Index into the entity enum.
	unsigned int x,y,bX,bY,hp,armour,thumbnail,class;
	void* state;
} BOX_Entity;

typedef struct _cameraPoint {
	int x,y;
} BOX_CameraPoint;

typedef struct _BOX_SignalHandler {
	int key;
	int item;
	struct _BOX_SignalHandler* next;
} BOX_SignalHandler;

typedef struct _BOX_ChunkEntity {
	int entitySpawner;
	const char* args;
	unsigned int x,y;
} BOX_ChunkEntity;

typedef struct _BOX_Chunk {
	uint32_t id;
	uint8_t bottom[CHUNKSIZE][CHUNKSIZE];
	uint8_t top[CHUNKSIZE][CHUNKSIZE];
	uint8_t clipping[CHUNKSIZE][CHUNKSIZE/8];
	uint8_t flags;
	BOX_ChunkEntity entities[CHUNK_ELIMIT];
} BOX_Chunk;

typedef struct _BOX_Message {
	BOX_Signal type; 
	union{
		/*BOX_SIGNAL_SPAWN*/
		struct {
			const char* spawnargs;
			BOX_Chunk* spawnchunk;
		};
		//BOX_SIGNAL_FRAME
		int frame;
		//BOX_SIGNAL_COLLISION
		int collider;
		//BOX_UPDATE_CAMERA
		BOX_CameraPoint camera;
		//BOX_POPULATE_CHUNK
		struct {
			BOX_Chunk** in;
			int sX,sY;
		};
	};
} BOX_Message;

#define MSG_SPAWN(args,chunk) (BOX_Message){.type=BOX_SIGNAL_SPAWN,.spawnargs=args,.spawnchunk=chunk}
#define MSG_FRAME(x) (BOX_Message){.type=BOX_SIGNAL_FRAME,.frame=x}
#define MSG_COLLIDER(x) (BOX_Message){.type=BOX_SIGNAL_COLLISION,.collider=x}
#define MSG_EMPTY (BOX_Message){.type=-1,.collider=-1}
#define MSG_CAMERAUPDATE(x,y) (BOX_Message){.type=BOX_UPDATE_CAMERA,.camera=(BOX_CameraPoint){x,y}}
#define MSG_POPULATE_CHUNK(chunk,sx,sy) (BOX_Message){.type=BOX_POPULATE_CHUNK,.in=chunk,.sX=sx,.sY=sy}

BOX_entId BOX_Camera();
BOX_CameraPoint BOX_CameraGet();
uint16_t BOX_Rand();
void BOX_SetSeed(uint32_t in);
uint16_t BOX_RandHash(int n);
unsigned int BOX_Diff (int val1, int val2);
unsigned int BOX_GetTicks();
int BOX_SendMessage(int sender, int target, BOX_Message in);
char BOX_CollisionCheck(BOX_Entity* in,int x, int y);
void BOX_SetCollision(BOX_Chunk* chunk, char x, char y, char z);
char BOX_GetKey(int codeIn);
void BOX_SetCamera(BOX_entId in);
BOX_entId BOX_NewEntityID();
uint16_t BOX_Hash(char *s);
unsigned int BOX_FrameCount();
BOX_SignalHandler* BOX_RegisterHandler(BOX_entId owner, int handler);
int BOX_RemoveEntity(int id);
BOX_Entity* BOX_GetEntity(int id);
BOX_Entity* BOX_GetEntityByTag(const char* tag);
int BOX_EntitySpawn(int in,const char* args, int x,int y);
int BOX_ChildEntitySpawn(int in, BOX_Entity* parent, const char* args, int x, int y);
int BOX_ChunkEntitySpawn(int in,const char* args, int x, int y, int sX, int sY);
void BOX_DrawBottom(int x, int y, int id);
void BOX_DrawTop(int x, int y, int id);
void BOX_RenderBGPage(BOX_Chunk* in);
void BOX_DrawBG();
void BOX_DrawColouredText(int x, int y, char* text, int r, int g, int b);
void BOX_DrawColouredRelativeText(int x, int y, char* text, int r, int g, int b);
void BOX_DrawText(int x, int y, char* text);
void BOX_DrawTextRelative(int x, int y, char* text);
void BOX_RenderList();
#endif
