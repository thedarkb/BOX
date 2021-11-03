#ifdef EDITOR
#include <tcl/tcl.h>
#include <tcl/tk.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <dlfcn.h>

#include "../engine.h"
#include "entities.h"

#define SELF BOX_GetEntity(receiver)
#define SCALEFACTOR 2
#define BIT(x) (1<<x)

int playerId=0;

Tcl_Interp* interp=NULL;
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

static void rect(int x, int y, int w, int h, uint8_t r, uint8_t g, uint8_t b, uint8_t a) {
	SDL_Rect out={x,y,w,h};
	SDL_SetRenderDrawColor(edRend,r,g,b,a);
	SDL_RenderFillRect(edRend,&out);
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

static void frameHandler(BOX_Signal signal, BOX_entId sender, BOX_entId receiver,void* state) {
	int mX, mY,button;
	static int debounce;
	if(!(BOX_FrameCount()%2)) return;

	SELF->x=BOX_CameraGet().x;
	SELF->y=BOX_CameraGet().y;
	if(!BOX_FrameCount()) return;

	if(target!=chunkCache[1][1]) {
		//TODO RESET EVERYTHING WHEN THIS HAPPENS.
		target=chunkCache[1][1];
		localMap=*chunkCache[1][1];
		localMap.clipping[0][1]=1;//TEMPORARY
	}

	if(button=SDL_GetMouseState(&mX,&mY)) {
		mY/=SCALEFACTOR;
		mX/=SCALEFACTOR;
		if(mX>CHUNKSIZE*TILESIZE+TILESIZE  && BOX_FrameCount()>debounce+10) {
			tileSelection+=mY/TILESIZE-16;
			Tcl_UpdateLinkedVar(interp,"tileNo");
		} else if(mX<CHUNKSIZE*TILESIZE && mY<CHUNKSIZE*TILESIZE) {
			int tileVal=0;
			switch(toolSelection) {
				case 0://Stamp tool.
					if(button&SDL_BUTTON(SDL_BUTTON_LEFT))
						tileVal=tileSelection;
					else if(button&SDL_BUTTON(SDL_BUTTON_RIGHT))
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
					if(button&SDL_BUTTON(SDL_BUTTON_LEFT))
						tileVal=tileSelection;
					else if(button&SDL_BUTTON(SDL_BUTTON_RIGHT))
						tileVal=0;
					if(layerSelection==0)
						fill(localMap.bottom,tileVal,localMap.bottom[mY/TILESIZE][mX/TILESIZE],mX/TILESIZE,mY/TILESIZE);
					else if(layerSelection==2)
						fill(localMap.top,tileVal,localMap.top[mY/TILESIZE][mX/TILESIZE],mX/TILESIZE,mY/TILESIZE);
				break;
						
				default:
				break;
			}
		}
		debounce=BOX_FrameCount();
	}

	draw(103,CHUNKSIZE*TILESIZE,CHUNKSIZE*TILESIZE/2);
	for(int i=0;i<32;i++) {
		draw(tileSelection-16+i,CHUNKSIZE*TILESIZE+TILESIZE,i*TILESIZE);
	}


	for(int y=0;y<CHUNKSIZE;y++) {
		for(int x=0;x<CHUNKSIZE;x++) {
			draw(localMap.bottom[y][x],x*TILESIZE,y*TILESIZE);
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
	if(layerSelection==4 || layerSelection==2) {
		for(int y=0;y<CHUNKSIZE;y++) {
			for(int x=0;x<CHUNKSIZE;x++) {
				draw(localMap.top[y][x],x*TILESIZE,y*TILESIZE);
			}
		}
	}
	if(layerSelection==4 || layerSelection==3 && target->initialiser) {
		localMap.initialiser(&localMap,spawnerShim);
	}


	if(Tcl_Eval(interp,"update\nlistboxen")!=TCL_OK){
		BOX_eprintf("%s\n",Tcl_GetStringResult(interp));
	}
	*target=localMap;
	SDL_RenderPresent(edRend);
	SDL_RenderClear(edRend);
}

static int refreshScript(ClientData clientData, Tcl_Interp* state, int argc, char** argv) {
	char filepath[255]={0};
	void* code=NULL;
	static void* oldHandle=NULL;
	void (*initialiserNext)(struct _BOX_Chunk* self, int(*spawner)(BOX_Entity*,int,int,int,int))=NULL;
	if(oldHandle) {
		dlclose(oldHandle);
	}

	if(argc<2) return 1;
	code=dlopen("./temp.so",RTLD_NOW);
	printf(dlerror());

	if(!code) {
		Tcl_Eval(interp,"tk_messageBox -message \"Compilation failed.\"");
		localMap.initialiser=NULL;
		return 0;
	}
	initialiserNext=dlsym(code,argv[1]);
	if(!initialiserNext) {
		Tcl_Eval(interp,"tk_messageBox -message \"Function not found, is its name the same as the filename excluding the extension?\"");
		localMap.initialiser=NULL;
		return 0;
	}
	oldHandle=code;
	localMap.initialiser=initialiserNext;

	return 0;
}

static int buildScript(ClientData clientData, Tcl_Interp* state, int argc, char** argv) {
	char filepath[255]={0};

	if(argc<2) return 1;

	strcat(filepath,"gcc -o temp.so -DSDK -shared -fPIC world/");
	strcat(filepath,argv[1]);
	strcat(filepath,".c -Wl,-R -Wl,. -L. -lentities");
	printf("%s\n",filepath);
	system(filepath);

	return 0;
}

static void setup(BOX_Signal signal, BOX_entId sender, BOX_entId receiver,void* state) {
	interp = Tcl_CreateInterp(); 
	if (Tcl_Init(interp) != TCL_OK) { 
		BOX_eprintf("Unable to start TCL interpreter, exiting.\n");
		return;
	}
	if(Tcl_EvalFile(interp, "mapeditor.tcl") != TCL_OK) {
		printf("Starting GUI event loop failed!\n");
		BOX_eprintf("%s\n",Tcl_GetStringResult(interp));
	}

	Tcl_LinkVar(interp,"tileNo",(char*)&tileSelection,TCL_LINK_INT);
	Tcl_LinkVar(interp,"layer",(char*)&layerSelection,TCL_LINK_INT);
	Tcl_LinkVar(interp,"tool",(char*)&toolSelection,TCL_LINK_INT);
	Tcl_CreateCommand(interp, "refreshScript",(void*)refreshScript, NULL, NULL);
	Tcl_CreateCommand(interp, "buildScript",(void*)buildScript, NULL, NULL);

	playerId=BOX_EntitySpawn(ent_player(77),6000,4000);
}

BOX_Entity* ent_editor() {
	static int ident;
	if(ident) BOX_panic("Attempt to spawn two map editors in one session.\n");
	BOX_Entity* me=NEW(BOX_Entity);
	memset(me,sizeof (BOX_Entity),0);
	BOX_RegisterHandler(BOX_SIGNAL_FRAME, BOX_NewEntityID(),frameHandler);
	BOX_RegisterHandler(BOX_SIGNAL_SPAWN, BOX_NewEntityID(),setup);
	
	SDL_RenderSetScale(r,2,2);
	SDL_SetWindowSize(w, 480, 320);

	edWin=SDL_CreateWindow(TITLE" Map Editor", 0, 640, TILESIZE*CHUNKSIZE*2+TILESIZE*4,TILESIZE*CHUNKSIZE*2, SDL_WINDOW_OPENGL);
	edRend=SDL_CreateRenderer(edWin,-1,SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_Surface* loader=IMG_Load("sheet.png");
	nsheet=SDL_CreateTextureFromSurface(edRend, loader);
	SDL_RenderSetScale(edRend,2,2);
	SDL_SetRenderDrawBlendMode(edRend,SDL_BLENDMODE_BLEND);
	
	ident=1;
	return me;
}
#endif