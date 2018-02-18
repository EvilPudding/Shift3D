CC = cc
LD = cc

DIR = build

LIBS = -Lcandle/build $(shell sdl2-config --libs) -lglut -lGLU -lm -lGL -lGLEW \
	   -lpng

SRCS = $(wildcard *.c) $(wildcard components/*.c) $(wildcard systems/*.c)

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))

LIBS_REL = $(LIBS) candle/build/candle.a

LIBS_DEB = $(LIBS) candle/build/candle_debug.a

CFLAGS = -Wall -I. -Icandle -DUSE_VAO \
		 $(shell sdl2-config --cflags)

CFLAGS_REL = $(CFLAGS) -O2

CFLAGS_DEB = $(CFLAGS) -g3

##############################################################################

all: update_lib init $(DIR)/shift
	cp -rvu resauces $(DIR)

update_lib:
	rm -f candle/build/candle.a

$(DIR)/shift: candle/build/candle.a $(OBJS_REL) 
	$(LD) -o $@ $(OBJS_REL) $(LIBS_REL)

candle/build/candle.a:
	$(MAKE) -C candle PARENTCFLAGS=

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: update_lib_deb init $(DIR)/shift_debug
	cp -rvu resauces $(DIR)

update_lib_deb:
	rm -f candle/build/candle_debug.a

$(DIR)/shift_debug: candle/build/candle_debug.a $(OBJS_DEB)
	$(LD) -o $@ $(OBJS_DEB) $(LIBS_DEB)

candle/build/candle_debug.a:
	$(MAKE) -C candle debug PARENTCFLAGS=

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components

##############################################################################

run: all
	cp -rvu resauces $(DIR)
	$(DIR)/shift 6

gdb: debug
	cp -rvu resauces $(DIR)
	gdb $(DIR)/shift_debug

valgrind: debug
	cp -rvu resauces $(DIR)
	valgrind --log-fd=1 --suppressions=val_sup $(DIR)/shift_debug | tee val_log | less
		
clean:
	rm -r $(DIR)
	$(MAKE) -C candle clean
