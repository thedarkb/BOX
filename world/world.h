#include "../entities/entities.h"
#define WORLD_CHUNKID(sX,sY) ((sY*65535)+sX)
#define WORLD_GETX(id) (id%65535)
#define WORLD_GETY(id) (id/65535)

BOX_Chunk* world_gen(unsigned int sX, unsigned int sY);