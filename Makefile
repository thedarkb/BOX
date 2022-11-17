CC = gcc
TITLE = take
CFLAGS = -g #-fsanitize=undefined,address
EMCCFLAGS = -O3 -s MODULARIZE=1 -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=2 -s USE_SDL_IMAGE=2 -s SDL2_IMAGE_FORMATS='["png"]' --embed-file sheet.png -o web/box.js

main:
	make world
	$(CC) main.c $(CFLAGS) -lSDL2 -lSDL2_image -Wl,-R -Wl,. -L. -lworld -lentities -o $(TITLE)
web:
	mkdir web
	cp index.html web/
	emcc main.c world/*.c entities/*.c $(EMCCFLAGS)
editor:
	#$(CC) -shared -o libworld.so $(CFLAGS)  -fPIC world/*.c
	#$(CC) -shared -DEDITOR $(CFLAGS) -ltcl -ltk8.6 -ltcc -ldl -o libentities.so -fPIC entities/*.c
	#$(CC) main.c -DEDITOR $(CFLAGS)  -ltcl -ltk8.6 -lSDL2 -lSDL2_image -Wl,-R -Wl,. -L. -lentities -lworld -ltcc -ldl -o $(TITLE)-editor
	$(CC) main.c world/*.c entities/*.c -DEDITOR $(CFLAGS) `pkg-config --cflags --libs gtk+-3.0 gdk-3.0` -rdynamic -lSDL2 -lSDL2_image -ldl -o $(TITLE)-editor
	
.PHONY: entities
entities:
	$(CC) -shared -o libentities.so $(CFLAGS) -fPIC entities/*.c
.PHONY: world
world:
	make entities
	$(CC) -shared -o libworld.so $(CFLAGS)  -fPIC world/*.c

.PHONY: clean
clean:
	rm libworld.so
	rm libentities.so
	rm take
	rm take-editor
	rm -r web
