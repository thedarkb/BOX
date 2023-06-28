/* If you're adding a new entity, append it to the end of this list.
 * The SPAWNER_LIST macro is used to automatically generate the function
 * prototypes along with the string name, the array of function pointers to
 * spawners, and the enumeration of spawners.*/

#define SPAWNER_LIST 	\
	SPAWNER_LIST_ITEM(ent_furniture) 	\
	SPAWNER_LIST_ITEM(ent_roundtree) 	\
	SPAWNER_LIST_ITEM(ent_player)		\
	SPAWNER_LIST_ITEM(ent_camera)		\
	SPAWNER_LIST_ITEM(ent_editor)		\
	SPAWNER_LIST_ITEM(ent_worldspawn)
