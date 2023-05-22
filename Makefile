CC = gcc
TITLE = take
CFLAGS = -g -fsanitize=undefined,address
EMCCFLAGS = -O3 -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --embed-file sheet.png -o web/box.js

main:
	make chunks
	make world
	$(CC) main.c $(CFLAGS) -lSDL2 -lSDL2_image -Wl,-R -Wl,. -L. -lworld -lentities -o $(TITLE)
web:
	make chunks
	mkdir web
	cp index.html web/
	emcc main.c world/*.c entities/*.c $(EMCCFLAGS)
editor:
	make chunks
	$(CC) main.c world/*.c entities/*.c -DEDITOR $(CFLAGS) `pkg-config --cflags --libs gtk+-3.0 gdk-3.0` -rdynamic -lSDL2 -lSDL2_image -ldl -o $(TITLE)-editor
	
.PHONY: entities
entities:
	$(CC) -shared -o libentities.so $(CFLAGS) -fPIC entities/*.c
.PHONY: world
world:
	make entities
	$(CC) -shared -o libworld.so $(CFLAGS)  -fPIC world/*.c
.PHONY: chunks
chunks:
	cat world/head.h world/chunk_*.h world/tail.h > world/chunks.h

.PHONY: clean
clean:
	rm libworld.so
	rm libentities.so
	rm take
	rm take-editor
	rm -r web
