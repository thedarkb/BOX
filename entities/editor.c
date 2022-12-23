#ifdef EDITOR
#include <gtk/gtk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dlfcn.h>

#include "../engine.h"
#include "entities.h"

#define ASYNC_BEGIN static int scrLine = 0; switch(scrLine) { case 0:
#define ASYNC_END scrLine=0;} return
#define YIELD do {scrLine=__LINE__;return; case __LINE__:;} while (0)

#define SELF BOX_GetEntity(receiver->id)
#define SCALEFACTOR 2.0f
#define BIT(x) (1<<x)

enum mapflags {
	WS_CHUNK,
	WS_MODIFIED,
};

int playerId=0;

extern int sX;
extern int sY;

extern size_t worldArray_len;
extern BOX_Chunk* worldArray;
extern BOX_Entity* entSet[ELIMIT];
extern unsigned int tileAnimClock;

BOX_Entity* ent_worldspawn(BOX_Chunk** self,unsigned int sX, unsigned int sY);

GtkBuilder* builder;
GObject* toolWin;

//Tcl_Interp* interp=NULL;
extern SDL_Renderer* r;
extern SDL_Window* w;
extern BOX_Chunk* chunkCache[3][3];
SDL_Texture* nsheet;

SDL_Window* edWin;
SDL_Renderer* edRend;
BOX_Chunk* target=NULL;
BOX_Chunk localMap;

int tileSelection=1;
int toolSelection=0;
int layerSelection=0;
int selectedTileX=-1;
int selectedTileY=-1;
int animated=0;

extern void chunkRefresh(void);

static void rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	SDL_Rect out={x,y,w,h};
	SDL_SetRenderDrawColor(edRend,r,g,b,a);
	SDL_RenderFillRect(edRend,&out);
	SDL_SetRenderDrawColor(edRend,0,0,0,255);
}

static void fill(uint8_t in[CHUNKSIZE][CHUNKSIZE], int id, int field, int x, int y) {
	if(x>CHUNKSIZE-1 || x<0) return;
	if(y>CHUNKSIZE-1 || y<0) return;
	if(in[y][x]!=field || in[y][x]==id) return;
	
	in[y][x]=id;
	fill(in,id,field,x-1,y);
	fill(in,id,field,x+1,y);
	fill(in,id,field,x,y-1);
	fill(in,id,field,x,y+1);
}

static void draw(int id, int x, int y) {
	SDL_Rect source={(id % (SPRITESHEET/TILESIZE))*TILESIZE, (id / (SPRITESHEET/TILESIZE))*TILESIZE,TILESIZE,TILESIZE};
	SDL_Rect dest={x,y,TILESIZE,TILESIZE};
	SDL_RenderCopy(edRend,nsheet,&source,&dest);
}

static int spawnerShim(BOX_Entity* in, int x, int y, int sX, int sY) {
	static int id;
	if(sX!=CAMERASX) return -1;
	if(sY!=CAMERASY) return -1;
	if(!in) {
		printf("Null passed to spawner!\n");
		return -1;
	}
	draw(in->thumbnail,x,y);
	return id++;
}

G_MODULE_EXPORT void openEntityList(GtkButton* button, gpointer data) {
	char* detokenised=malloc(CHUNK_ELIMIT*512);
	detokenised[0]='\0';
	gtk_widget_show(GTK_WIDGET(gtk_builder_get_object(builder,"entListWin")));
	for(int i=0;i<CHUNK_ELIMIT;i++) {
		char line[512];
		if(localMap.entities[i].entitySpawner>0) {
			if(localMap.entities[i].args)
				sprintf(line,"%d,%d,%s:%s\n",localMap.entities[i].x,localMap.entities[i].y,editor_entities[localMap.entities[i].entitySpawner](0,0,"",NULL)->tooltip,localMap.entities[i].args);
			else
				sprintf(line,"%d,%d,%s:\n",localMap.entities[i].x,localMap.entities[i].y,editor_entities[localMap.entities[i].entitySpawner](0,0,"",NULL)->tooltip);
			strcat(detokenised,line);
		}
		else
			strcat(detokenised,"\n");
		
	}
	gtk_text_buffer_set_text(GTK_TEXT_BUFFER(gtk_builder_get_object(builder,"entityListText")),(const gchar*)detokenised,-1);
}

G_MODULE_EXPORT void hideEntities(GtkButton* button, gpointer data) {
	gtk_widget_hide(GTK_WIDGET(gtk_builder_get_object(builder,"entListWin")));	
}

G_MODULE_EXPORT void tokeniseEntities(GtkButton* button, gpointer data) {
	int c=0;
	char *text;
	char *token;
	char *list[CHUNK_ELIMIT];
	GtkTextIter start,end;
	
	for(int i=0;i<CHUNK_ELIMIT;i++) {
		localMap.entities[i].entitySpawner=-1;
		if(localMap.entities[i].args)
			free((void*)localMap.entities[i].args);
		localMap.entities[i].args=NULL;
		localMap.entities[i].x=localMap.entities[i].y=0;
	}
	
	gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(gtk_builder_get_object(builder,"entityListText")), &start);
	gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(gtk_builder_get_object(builder,"entityListText")), &end);
	text=gtk_text_buffer_get_text(GTK_TEXT_BUFFER(gtk_builder_get_object(builder,"entityListText")), &start, &end, FALSE);
	token=strtok(text,"\n");
	
	if(!token) 		
		return;

	while (token!=NULL && c<CHUNK_ELIMIT) {
		list[c++]=token;
		printf("%s\n",token);
		token=strtok(NULL,"\n");
	}
	for(int i=0;i<CHUNK_ELIMIT && i<c;i++) {
		char entityName[255];
		int tX,tY;
		int spawnId=-1;
		
		token=strtok(list[i],":");
		if(!token)
			continue;
		if(sscanf(token,"%d,%d,%s",&tX,&tY,entityName)!=3) {
			sprintf(entityName,"Invalid spawn definition:\n\n%s\n\nExpected <X Tile>,<Y Tile>,<Entity Type>:<Arguments>",token);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Spawn Definition Error", (const char*)&entityName[0], edWin);
			continue;		
		}
		for(int j=1;j<EDITOR_ENTCOUNT;j++) {
			if(!strcmp(editor_entities[j](0,0,"",&localMap)->tooltip,entityName)) {
				spawnId=j;
				break;				
			}
		}
		if(spawnId==-1) {
			char errorMsg[255];
			sprintf(errorMsg,"Entity \"%s\" not found.",entityName);
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Spawn Definition Error", (const char*)&errorMsg[0], edWin);
			continue;		
		}
		localMap.entities[i].x=tX;
		localMap.entities[i].y=tY;
		localMap.entities[i].entitySpawner=spawnId;
		token=strtok(NULL,":");
		if(token) {
			for(int j=0;token[j];j++) {
				if(token[j]!=' ') {
					localMap.entities[i].args=malloc(strlen(token)+1);
					strcpy((char *__restrict)localMap.entities[i].args,(const char*)token);					
				}				
			}
		}
	}
}
G_MODULE_EXPORT void deleteChunk(GtkButton* button, gpointer data) {
	for(int i=0;i<worldArray_len;i++) {
		if(worldArray[i].id==localMap.id) {
			worldArray[i].id=UINT32_MAX;
			worldArray[i].flags=0;
		}
	}
	localMap.id=UINT32_MAX;
	BOX_EntitySpawn(ent_worldspawn(&target,sX,sY),0,0);
	localMap=*target;
	localMap.flags=0;
}

G_MODULE_EXPORT void killEntities(GtkButton* button, gpointer data) {
	for(int i=3;i<ELIMIT;i++) {//This is dirty and works off the assumption that the editor is in slot 0, the player in slot 1, and the camera in slot 2.
		if(entSet[i]){
			if(entSet[i]->state) {
				free(entSet[i]->state);				
			}
			free(entSet[i]);
			entSet[i]=NULL;
		}
	}	
}

G_MODULE_EXPORT void spawnMapEntities(GtkButton* button, gpointer data) {
	for(int i=0;i<CHUNK_ELIMIT;i++) {
		if(localMap.entities[i].entitySpawner>-1) {
			if(localMap.entities[i].args) {
				BOX_ChunkEntitySpawn(
					editor_entities[localMap.entities[i].entitySpawner](
						sX*CHUNKSIZE+localMap.entities[i].x,
						sY*CHUNKSIZE+localMap.entities[i].y,
						localMap.entities[i].args,
						&localMap
					),
					localMap.entities[i].x*TILESIZE,
					localMap.entities[i].y*TILESIZE,
					sX,
					sY
				);
			} else {
				BOX_ChunkEntitySpawn(
					editor_entities[localMap.entities[i].entitySpawner](
						sX*CHUNKSIZE+localMap.entities[i].x,
						sY*CHUNKSIZE+localMap.entities[i].y,
						NULL,
						&localMap
					),
					localMap.entities[i].x*TILESIZE,
					localMap.entities[i].y*TILESIZE,
					sX,
					sY
				);
			}
		}
	}	
}

G_MODULE_EXPORT void screenChange(GtkButton* button, gpointer data) {
	//This is really dirty, it just sends the player to the selected chunk so the camera and the editor follow.
	int diffSx=gtk_adjustment_get_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"sX")));
	int diffSy=gtk_adjustment_get_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"sY")));
	
	if(diffSx<sX) {
		BOX_GetEntity(1)->x=diffSx*CHUNKSIZE*TILESIZE+CHUNKSIZE/2*TILESIZE;
	} else if(diffSx>sX) {
		BOX_GetEntity(1)->x=diffSx*CHUNKSIZE*TILESIZE+CHUNKSIZE/2*TILESIZE;
	}
	if(diffSy<sY) {
		BOX_GetEntity(1)->y=diffSy*CHUNKSIZE*TILESIZE+CHUNKSIZE/2*TILESIZE;
	} else if(diffSy>sY) {
		BOX_GetEntity(1)->y=diffSy*CHUNKSIZE*TILESIZE+CHUNKSIZE/2*TILESIZE;
	}
}

G_MODULE_EXPORT void chunkDumper(GtkButton* button, gpointer data) {
	int new_len=0;
	FILE* out;
	char filePath[32];
	system("bash -c 'mv world/chunk_*.h world/oldchunks/'");
	for(int h=0;h<worldArray_len;h++) {
		if(worldArray[h].id<0)
			continue;
		if(worldArray[h].id==UINT32_MAX)
			continue;
		if(!(worldArray[h].flags&BIT(WS_MODIFIED)) && !(worldArray[h].flags&BIT(WS_CHUNK)))
			continue;
			
		sprintf(filePath,"world/chunk_%d.h",worldArray[h].id);
			
		out=fopen(filePath,"w");
		if(!out) {
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Error Writing File", "Unable to write to write to chunk file.", edWin);
			return;
		}
		
		new_len++;
		fprintf(out,"\t{\n");
		fprintf(out,"\t\t%d,\n",worldArray[h].id);
		fprintf(out,"/*Bottom Layer*/\n");
		fprintf(out,"\t\t{\n");
		for(int y=0;y<CHUNKSIZE;y++) {
			fprintf(out,"\t\t\t{");
			for(int x=0;x<CHUNKSIZE;x++) {
				fprintf(out,"%d,",worldArray[h].bottom[y][x]);
			}
			fprintf(out,"},\n");
		}
		fprintf(out,"\t\t},\n");
		fprintf(out,"/*Top Layer*/\n");
		fprintf(out,"\t\t{\n");
		for(int y=0;y<CHUNKSIZE;y++) {
			fprintf(out,"\t\t\t{");
			for(int x=0;x<CHUNKSIZE;x++) {
				fprintf(out,"%d,",worldArray[h].top[y][x]);
			}
			fprintf(out,"},\n");
		}
		fprintf(out,"\t\t},\n");
		fprintf(out,"/*Clipping Layer*/\n");
		fprintf(out,"\t\t{\n");
		for(int y=0;y<CHUNKSIZE;y++) {
			fprintf(out,"\t\t\t{");
			for(int x=0;x<CHUNKSIZE/8;x++) {
				fprintf(out,"%d,",worldArray[h].clipping[y][x]);
			}
			fprintf(out,"},\n");
		}
		fprintf(out,"\t\t},\n");
		fprintf(out,"\t\t%d,//Flags\n",worldArray[h].flags);
		fprintf(out,"/*Entity Spawns*/\n");
		fprintf(out,"\t\t{\n");
		for(int i=0;i<CHUNK_ELIMIT;i++) {
			if(worldArray[h].entities[i].args)
				fprintf(out,"\t\t\t{%d,\"%s\",%d,%d},\n",worldArray[h].entities[i].entitySpawner,worldArray[h].entities[i].args,worldArray[h].entities[i].x,worldArray[h].entities[i].y);
			else
				fprintf(out,"\t\t\t{%d,NULL,%d,%d},\n",worldArray[h].entities[i].entitySpawner,worldArray[h].entities[i].x,worldArray[h].entities[i].y);
		}
		fprintf(out,"\t\t}\n");
		fprintf(out,"\t},\n");
		fclose(out);
	}
	out=fopen("world/tail.h","w");
	fprintf(out,"};\n");
	fprintf(out,"\nsize_t worldArray_len=%d;\n\n",new_len);
	fprintf(out,"BOX_Chunk* worldArray=&_worldArray[0];\n");
	fclose(out);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Operation Successful", "World has been written to chunk files.", edWin);
}

static void cloneVerifyLocalmap() {
	char statflag=0;

	for(int i=0;i<worldArray_len;i++) {
		if(worldArray[i].id==localMap.id) {
			worldArray[i]=localMap;
			return;
			statflag=1;
		}
	}
	if(!statflag) {
		worldArray_len++;
		worldArray=realloc(worldArray,worldArray_len*sizeof(worldArray[0]));
		worldArray[worldArray_len-1]=localMap;
		chunkRefresh();
	}
	
	target=chunkCache[1][1];
	localMap=*chunkCache[1][1];
	
	for(int j=0;j<CHUNK_ELIMIT;j++) {
		char* newString=malloc(strlen(localMap.entities[j].args)+1);
		strcpy(newString,localMap.entities[j].args);
		localMap.entities[j].args=(const char*)newString;
	}
}

static void frameHandler(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	int mX, mY,button;
	static int debounce;
	char messages[255];
	
	for(int i=0;i<4;i++)
		gtk_main_iteration_do(FALSE);
	
	//printf("%d\n",localMap.flags);
			
	ASYNC_BEGIN;

	SELF->x=BOX_CameraGet().x;
	SELF->y=BOX_CameraGet().y;
	if(!BOX_FrameCount()) return;
	
	/*Handle a change in the active chunk.*/
	if(target!=chunkCache[1][1]) {		
		target=chunkCache[1][1];
		localMap=*chunkCache[1][1];
		
		gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"sX")),sX);
		gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"sY")),sY);
	}
	/*End of active chunk change handling.*/
	
	/*Propagate changes made in GTK toolbox to SDL editing window.*/
	tileSelection=gtk_adjustment_get_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"tileNo")));
	toolSelection=gtk_combo_box_get_active(GTK_COMBO_BOX(gtk_builder_get_object(builder,"toolSel")));
	layerSelection=gtk_combo_box_get_active(GTK_COMBO_BOX(gtk_builder_get_object(builder,"layerSel")));
	selectedTileX=gtk_adjustment_get_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"selX")));
	selectedTileY=gtk_adjustment_get_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"selY")));
	animated=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(builder,"animToggle")));
	
	if((localMap.flags&BIT(WS_MODIFIED)) || (localMap.flags&BIT(WS_CHUNK))) {
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(builder,"chunkDelete")),TRUE);
	} else {
		gtk_widget_set_sensitive(GTK_WIDGET(gtk_builder_get_object(builder,"chunkDelete")),FALSE);
	}
	sprintf(messages,"%d",localMap.id);
	gtk_label_set_text(GTK_LABEL(gtk_builder_get_object(builder,"chunkNum")),(const char*)&messages[0]);
	/*-----------------------------------------------------------*/
	
	/*Handle clicks in the editing window.*/
	if(button=SDL_GetMouseState(&mX,&mY)) {
		mY/=SCALEFACTOR;
		mX/=SCALEFACTOR;
		if(mX>CHUNKSIZE*TILESIZE+TILESIZE  && BOX_FrameCount()>debounce+10) {
			tileSelection+=mY/TILESIZE-CHUNKSIZE/2;
			gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"tileNo")),tileSelection);
			
		} else if(mX<CHUNKSIZE*TILESIZE && mY<CHUNKSIZE*TILESIZE) {
			int tileVal=0;
			switch(toolSelection) {
				case 0://Stamp tool.
					localMap.flags|=BIT(WS_MODIFIED);
					if(button&SDL_BUTTON(SDL_BUTTON_LEFT)) {
						tileVal=tileSelection;
						if(animated)
							tileVal|=ANIMFLAG;
					} else if(button&SDL_BUTTON(SDL_BUTTON_RIGHT))
						tileVal=0;
					if(layerSelection==0)
						localMap.bottom[mY/TILESIZE][mX/TILESIZE]=tileVal;
					else if(layerSelection==2)
						localMap.top[mY/TILESIZE][mX/TILESIZE]=tileVal;
					else if(layerSelection==1) {
						if(tileVal)
							localMap.clipping[mY/TILESIZE][mX/TILESIZE/8]|=BIT(mX/TILESIZE%8);
						else
							localMap.clipping[mY/TILESIZE][mX/TILESIZE/8]&=~BIT(mX/TILESIZE%8);
					}
				break;
				case 1://Fill tool.
					localMap.flags|=BIT(WS_MODIFIED);
					if(button&SDL_BUTTON(SDL_BUTTON_LEFT))
						tileVal=tileSelection;
					else if(button&SDL_BUTTON(SDL_BUTTON_RIGHT))
						tileVal=0;
					if(layerSelection==0)
						fill(localMap.bottom,tileVal,localMap.bottom[mY/TILESIZE][mX/TILESIZE],mX/TILESIZE,mY/TILESIZE);
					else if(layerSelection==2)
						fill(localMap.top,tileVal,localMap.top[mY/TILESIZE][mX/TILESIZE],mX/TILESIZE,mY/TILESIZE);
				break;
				case 2://Select tool.
					if(button&SDL_BUTTON(SDL_BUTTON_LEFT)) {
						selectedTileX=mX/TILESIZE;
						selectedTileY=mY/TILESIZE;
						gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"selX")),selectedTileX);
						gtk_adjustment_set_value(GTK_ADJUSTMENT(gtk_builder_get_object(builder,"selY")),selectedTileY);
					}
				break;
						
				default:
				break;
			}
		}
		debounce=BOX_FrameCount();
	}
	/*------------------------------------*/

	/*Draw tile pallet.*/
	draw(103,CHUNKSIZE*TILESIZE,CHUNKSIZE*TILESIZE/2);
	for(int i=0;i<CHUNKSIZE;i++) {
		draw(tileSelection-CHUNKSIZE/2+i,CHUNKSIZE*TILESIZE+TILESIZE,i*TILESIZE);
	}
	/*----------------*/
	
	/*Draw each of the map layers in turn, depending on selection.*/
	for(int y=0;y<CHUNKSIZE;y++) {
		for(int x=0;x<CHUNKSIZE;x++) {
			if(localMap.bottom[y][x]&ANIMFLAG)
				draw((localMap.bottom[y][x]+tileAnimClock)&TILEMASK,x*TILESIZE,y*TILESIZE);
			else
				draw(localMap.bottom[y][x]&TILEMASK,x*TILESIZE,y*TILESIZE);
		}
	}
	if(layerSelection==1) {
		for(int y=0;y<CHUNKSIZE;y++) {
			for(int x=0;x<CHUNKSIZE;x++) {
				if(localMap.clipping[y][x/8]&BIT(x%8))
					rect(x*TILESIZE,y*TILESIZE,TILESIZE,TILESIZE,0,0,0,200);
			}
		}
	}
	else if(layerSelection!=4 && layerSelection!=0) {
		rect(0,0,CHUNKSIZE*TILESIZE,CHUNKSIZE*TILESIZE,0,0,0,200);
	}
	YIELD;
	if(layerSelection==4 || layerSelection==2) {
		for(int y=0;y<CHUNKSIZE;y++) {
			for(int x=0;x<CHUNKSIZE;x++) {
				if(localMap.top[y][x]&ANIMFLAG)
					draw((localMap.top[y][x]+tileAnimClock)&TILEMASK,x*TILESIZE,y*TILESIZE);
				else
					draw(localMap.top[y][x]&TILEMASK,x*TILESIZE,y*TILESIZE);
			}
		}
	}
	if((layerSelection==4 || layerSelection==3) /*&& localMap.initialiser*/) {
		for(int i=0;i<CHUNK_ELIMIT;i++) {
			if(localMap.entities[i].entitySpawner>-1) {
				BOX_Entity* temp=editor_entities[localMap.entities[i].entitySpawner](localMap.entities[i].x,localMap.entities[i].y,localMap.entities[i].args,&localMap);
				draw(temp->thumbnail,localMap.entities[i].x*TILESIZE,localMap.entities[i].y*TILESIZE);
				free(temp);
			}
		}
	}
	/*----------------------------------------------------------*/
	
	if(toolSelection==2) { //Draw a square on selected tile when select selected.
		rect(selectedTileX*TILESIZE,selectedTileY*TILESIZE,TILESIZE,TILESIZE,150,150,255,200);
	}
	
	
	*target=localMap;
	cloneVerifyLocalmap();
	
	SDL_RenderPresent(edRend);
	SDL_RenderClear(edRend);
	ASYNC_END;
}

static void setup(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	BOX_Chunk* oldWorldArray=worldArray;
	
	worldArray=malloc(sizeof(BOX_Chunk)*worldArray_len);
	memcpy(worldArray,oldWorldArray,worldArray_len*sizeof(BOX_Chunk));
	for(int i=0;i<worldArray_len;i++) {//Fixes up the entity argument strings so they can be edited.
		for(int j=0;j<CHUNK_ELIMIT;j++) {
			if(!worldArray[i].entities[j].args)
				continue;
			char* newString=malloc(strlen(worldArray[i].entities[j].args)+1);
			strcpy(newString,worldArray[i].entities[j].args);
			worldArray[i].entities[j].args=(const char*)newString;
		}
	}
	
	gtk_init(NULL,NULL);
	builder=gtk_builder_new();
	gtk_builder_add_from_file(builder,"toolbox.glade",NULL);
	
	toolWin=gtk_builder_get_object(builder,"toolWin");
	gtk_widget_show(GTK_WIDGET(toolWin));
    gtk_builder_connect_signals(builder, NULL);	

	playerId=BOX_EntitySpawn(ent_player(77),6000,4000);
}

static void signalSwitchboard(BOX_Signal signal, BOX_Entity* sender, BOX_Entity* receiver,BOX_Message state) {
	switch(signal) {
		case BOX_SIGNAL_FRAME:
			frameHandler(signal,sender,receiver,state);
		break;
		case BOX_SIGNAL_SPAWN:
			setup(signal,sender,receiver,state);
		break;
	}		
}

BOX_Entity* ent_editor() {
	static int ident;
	if(ident) BOX_panic("Attempt to spawn two map editors in one session.\n");
	BOX_Entity* me=NEW(BOX_Entity);
	memset(me,sizeof (BOX_Entity),0);
	me->postbox=signalSwitchboard;
	me->tag=NULL;
	
	SDL_RenderSetScale(r,2,2);
	SDL_SetWindowSize(w, 480, 320);

	edWin=SDL_CreateWindow(TITLE" Map Editor", 0, 640, TILESIZE*CHUNKSIZE*SCALEFACTOR+TILESIZE*4,TILESIZE*CHUNKSIZE*SCALEFACTOR, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
	edRend=SDL_CreateRenderer(edWin,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Surface* loader=IMG_Load("sheet.png");
	nsheet=SDL_CreateTextureFromSurface(edRend, loader);
	SDL_RenderSetScale(edRend,SCALEFACTOR,SCALEFACTOR);
	SDL_SetRenderDrawBlendMode(edRend,SDL_BLENDMODE_BLEND);
	
	ident=1;
	return me;
}
#endif
