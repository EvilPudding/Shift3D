CC = cc
LD = cc
AR = ar

emscripten: CC = emcc
emscripten: LD = emcc
emscripten: AR = emar

DIR = build

SRCS = $(wildcard *.c) $(wildcard components/*.c)

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))
OBJS_EMS = $(patsubst %.c, $(DIR)/%.emscripten.o, $(SRCS))

CFLAGS = -Wall -Wno-unused-function

CFLAGS_REL = $(CFLAGS) -std=c99 -O3

CFLAGS_DEB = $(CFLAGS) -std=c99 -g3 -DDEBUG

CFLAGS_EMS = $(CFLAGS) -O3

##############################################################################

all: init $(DIR)/coolbutts

release: all
	make -C candle SAUCES=resauces

$(DIR)/coolbutts: $(OBJS_REL)
	$(LD) -o $@ $(OBJS_REL) `candle/candle-config --libs`

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: init $(DIR)/coolbutts_debug
	make -C candle debug SAUCES=resauces

$(DIR)/coolbutts_debug: $(OBJS_DEB)
	$(LD) -o $@ $(OBJS_DEB) `candle/candle-config --debug --libs`

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

emscripten: init $(DIR)/index.js
	make -C candle emscripten SAUCES=resauces
	$(CC) -o index.html $(OBJS_EMS)

$(DIR)/index.js: $(OBJS_EMS)
	$(LD) -o $@ `candle/candle-config --emscripten --libs`

$(DIR)/%.emscripten.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_EMS)

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components

##############################################################################

run: all
	$(DIR)/coolbutts 11

gdb: debug
	gdb $(DIR)/coolbutts_debug

valgrind: debug
	valgrind --log-fd=1 --suppressions=val_sup $(DIR)/coolbutts_debug 10 | tee val_log | less
		
clean:
	rm -r $(DIR)
	$(MAKE) -C candle clean

.PHONY: init clean release
