#ifndef ENT_H
#define ENT_H
BOX_Entity* ent_furniture(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_roundtree(int,int,const char*,BOX_Chunk*);
BOX_Entity* ent_player(char);
BOX_Entity* ent_camera(int);
BOX_Entity* ent_editor();

#ifdef EDENTS
BOX_Entity* (*editor_entities[])(int,int,const char*,BOX_Chunk*)={
	NULL,
	ent_furniture,
	ent_roundtree
};

#else
extern BOX_Entity* (*editor_entities[])(int,int,const char*,BOX_Chunk*);
#endif
#define EDITOR_ENTCOUNT 3

#endif
