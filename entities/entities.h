#ifndef ENT_H
#define ENT_H
/*
BOX_Entity* ent_furniture(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_roundtree(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_player(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_camera(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_editor(int,int,const char*,BOX_Chunk*);
*/

#ifndef EDITOR
#define ent_editor ent_dummy
#endif

#include "spawner_list.h"
	
//Create spawner Function Prototypes.
#define SPAWNER_LIST_ITEM(y) void y(BOX_Signal, BOX_Entity*, BOX_Entity*,BOX_Message);
	SPAWNER_LIST
#undef SPAWNER_LIST_ITEM

//Enumerate previously mentioned arrays.
#define SPAWNER_LIST_ITEM(y) SPAWN_##y,
	enum {
		SPAWNER_LIST
		SPAWNER_LIST_COUNT
	};
#undef SPAWNER_LIST_ITEM

#endif
