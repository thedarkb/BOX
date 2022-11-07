#ifndef ENT_H
#define ENT_H
BOX_Entity* ent_furniture(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_player(char);
BOX_Entity* ent_camera(int);
BOX_Entity* ent_editor();

#ifdef EDENTS
BOX_Entity* (*editor_entities[])(int,int,const char*,BOX_Chunk*)={
	ent_furniture,
};

#else
extern BOX_Entity* (*editor_entities[])(int,int,const char*,BOX_Chunk*);
#endif
#define EDITOR_ENTCOUNT 1

#endif
