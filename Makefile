CC = gcc
TITLE = take
CFLAGS = -g -Ofast #-fsanitize=undefined

main:
	make world
	$(CC) main.c $(CFLAGS) -lSDL2 -lSDL2_image -Wl,-R -Wl,. -L. -lworld -lentities -o $(TITLE)

editor:
	$(CC) -shared -o libworld.so $(CFLAGS)  -fPIC world/*.c
	$(CC) -shared -DEDITOR $(CFLAGS) -ltcl -ltk8.6 -ltcc -ldl -o libentities.so -fPIC entities/*.c
	$(CC) main.c -DEDITOR $(CFLAGS)  -ltcl -ltk8.6 -lSDL2 -lSDL2_image -Wl,-R -Wl,. -L. -lentities -lworld -ltcc -ldl -o $(TITLE)-editor

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