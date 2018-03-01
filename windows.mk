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

LIBS_REL = $(LIBS) candle/build/libcandle.dll.a

LIBS_DEB = $(LIBS) candle/build/libcandle_debug.a

CFLAGS = -Wall -Icandle -DUSE_VAO

CFLAGS_REL = $(CFLAGS) -O2

CFLAGS_DEB = $(CFLAGS) -g3

##############################################################################

all: update_lib init $(DIR)/shift
	cp -rvu ../resauces $(DIR)

update_lib:
	rm -f candle/build/libcandle.a

$(DIR)/shift: candle/build/libcandle.a $(OBJS_REL) 
	$(LD) -o $@ $(OBJS_REL) $(LIBS_REL)

candle/build/libcandle.a:
	(cd candle && $(MAKE) -f windows.mk)

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: update_lib_deb init $(DIR)/shift_debug
	cp -rvu ../resauces $(DIR)

update_lib_deb:
	rm -f candle/build/libcandle_debug.a

$(DIR)/shift_debug: candle/build/libcandle_debug.a $(OBJS_DEB)
	$(LD) -o $@ $(OBJS_DEB) $(LIBS_DEB)

candle/build/libcandle_debug.a:
	$(MAKE) -C candle debug

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components

##############################################################################

run: all
	cp -rvu ../resauces $(DIR)
	$(DIR)/shift 6

gdb: debug
	cp -rvu ../resauces $(DIR)
	gdb $(DIR)/shift_debug

valgrind: debug
	cp -rvu ../resauces $(DIR)
	valgrind --suppressions=val_sup $(DIR)/shift_debug

clean:
	rm -r $(DIR)
	$(MAKE) -C candle clean
