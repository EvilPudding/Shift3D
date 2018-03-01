CC = gcc
LD = gcc

DIR = build

LIBS = \
       -lm -lmingw32 -lSDL2main -lSDL2 \
       /mingw/lib/libfreeglut.a \
       /mingw/lib/libglew32.a \
       /mingw/lib/libopengl32.a


SRCS = $(wildcard *.c) $(wildcard components/*.c) $(wildcard systems/*.c)

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))

LIBS_REL = candle/build/candle.a $(LIBS)

LIBS_DEB = candle/build/candle_debug.a $(LIBS)

CFLAGS = -Wall -Icandle -DUSE_VAO

CFLAGS_REL = $(CFLAGS) -O2

CFLAGS_DEB = $(CFLAGS) -g3

##############################################################################

all: update_lib init $(DIR)/shift
	cp -rvu resauces $(DIR)
	cp -rvu candle/SDL2.dll build

update_lib:
	rm -f candle/build/candle.a

$(DIR)/shift: candle/build/candle.a $(OBJS_REL) 
	$(LD) -o $@ $(OBJS_REL) $(LIBS_REL)

candle/build/candle.a:
	(cd candle && $(MAKE) -f windows.mk)

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: update_lib_deb init $(DIR)/shift_debug
	cp -rvu resauces $(DIR)
	cp -rvu candle/SDL2.dll $(DIR)

update_lib_deb:
	rm -f candle/build/candle_debug.a

$(DIR)/shift_debug: candle/build/candle_debug.a $(OBJS_DEB)
	$(LD) -o $@ $(OBJS_DEB) $(LIBS_DEB)

candle/build/candle_debug.a:
	(cd candle && $(MAKE) -f windows.mk debug)

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components

##############################################################################

run: all
	cp -rvu resauces $(DIR)
	cp -rvu candle/SDL2.dll $(DIR)
	$(DIR)/shift 6

gdb: debug
	cp -rvu resauces $(DIR)
	cp -rvu candle/SDL2.dll $(DIR)
	gdb $(DIR)/shift_debug

valgrind: debug
	cp -rvu resauces $(DIR)
	cp -rvu candle/SDL2.dll $(DIR)
	valgrind --suppressions=val_sup $(DIR)/shift_debug

clean:
	rm -r $(DIR)
	$(MAKE) -C candle clean
