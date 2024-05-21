CC = gcc
CFLAGS = -Wall -lpthread -Iinclude -std=gnu99
MAIN_OBJ = obj/main.o
OBJS = obj/myutil.o obj/fileops.o

all: main

main: $(MAIN_OBJ) $(OBJS)
	$(CC) -o MWCp $(MAIN_OBJ) $(OBJS) $(CFLAGS)

obj/%.o: src/%.c
	$(CC) -c -o $@ $< $(CFLAGS)

clean:
	rm -rf $(MAIN_OBJ)
	rm -rf $(OBJS)
	rm -rf main