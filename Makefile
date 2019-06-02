CC = cc
LD = cc
AR = ar

emscripten: CC = emcc
emscripten: LD = emcc
emscripten: AR = emar

PLUGINS = $(wildcard *.candle) candle

DIR = build

LIBS = 

SRCS = $(wildcard *.c) $(wildcard components/*.c)

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))

PLUGINS_REL = $(patsubst %, %/build/export.a, $(PLUGINS))
PLUGINS_DEB = $(patsubst %, %/build/export_debug.a, $(PLUGINS))
PLUGINS_EMS = $(patsubst %, %/build/export_emscripten.a, $(PLUGINS))

LIBS_REL = $(LIBS) $(PLUGINS_REL)
LIBS_DEB = $(LIBS) $(PLUGINS_DEB)
LIBS_EMS = $(LIBS) $(PLUGINS_EMS) $(EMS_OPTS) -O2

CFLAGS = -Wall -I. -Icandle -DUSE_VAO \
		 $(shell sdl2-config --cflags)

CFLAGS_REL = $(CFLAGS) -O3

CFLAGS_DEB = $(CFLAGS) -g3 -DDEBUG

CFLAGS_EMS = $(CFLAGS) -O3 -s USE_SDL=2 -s ALLOW_MEMORY_GROWTH=1 -s USE_WEBGL2=1 \
		     -s FULL_ES3=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0 -s EMULATE_FUNCTION_POINTER_CASTS=1 -s SAFE_HEAP=1 \
		     -s WASM=1


##############################################################################

all: init $(DIR)/coolbutts

release: all
	tar -cf $(DIR)/archive.tar -C $(DIR) $(RES) coolbutts
	gzip -9f $(DIR)/archive.tar
	cp candle/selfextract.sh $(DIR)/coolbutts.sh
	cat $(DIR)/archive.tar.gz >> $(DIR)/coolbutts.sh
	rm $(DIR)/archive.tar.gz
	mkdir -p release
	mv $(DIR)/coolbutts.sh release/coolbutts
	chmod +x release/coolbutts

$(DIR)/coolbutts: $(OBJS_REL) $(PLUGINS_REL)
	$(LD) -o $@ $(OBJS_REL) $(LIBS_REL) $(shell cat $(DIR)/deps)

%/build/export.a:
	$(MAKE) -C $(patsubst %/build/export.a, %, $@)
	echo " " >> $(DIR)/deps
	-cat $(patsubst %/build/export.a, %/build/deps, $@) >> $(DIR)/deps

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: init $(DIR)/shift_debug

$(DIR)/shift_debug: $(OBJS_DEB) $(PLUGINS_DEB)
	$(LD) -o $@ $(OBJS_DEB) $(LIBS_DEB) $(shell cat $(DIR)/deps)

%/build/export_debug.a:
	$(MAKE) -C $(patsubst %/build/export_debug.a, %, $@) debug
	echo " " >> $(DIR)/deps
	-cat $(patsubst %/build/export_debug.a, %/build/deps, $@) >> $(DIR)/deps

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

emscripten: init $(DIR)/index.js
	mkdir -p release
	cp candle/index.html build
	rm -f release/index.zip
	zip release/index.zip $(DIR)/index.*

$(DIR)/index.js: $(OBJS_EMS) $(PLUGINS_EMS)
	$(LD) -o $@ $(OBJS_EMS) $(LIBS_EMS) $(shell cat $(DIR)/deps) --preload-file resauces \
		--preload-file default.vil --preload-file transparent.vil $(CFLAGS_EMS)

%/build/export_emscripten.a:
	$(MAKE) -C $(patsubst %/build/export_emscripten.a, %, $@) emscripten
	echo " " >> $(DIR)/deps
	-cat $(patsubst %/build/export_emscripten.a, %/build/deps, $@) >> $(DIR)/deps

$(DIR)/%.emscripten.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_EMS)

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components
	rm -f $(DIR)/deps
	touch $(DIR)/deps
	rm -f $(PLUGINS_REL)
	rm -f $(PLUGINS_DEB)
	rm -f $(PLUGINS_EMS)
	cp -vu candle/*.vil $(DIR)
	cp -rvu resauces $(DIR)

##############################################################################

run: all
	cp -rvu resauces $(DIR)
	$(DIR)/coolbutts 11

gdb: debug
	cp -rvu resauces $(DIR)
	gdb $(DIR)/shift_debug

valgrind: debug
	cp -rvu resauces $(DIR)
	valgrind --log-fd=1 --suppressions=val_sup $(DIR)/shift_debug 10 | tee val_log | less
		
clean:
	rm -r $(DIR)
	rm -fr release
	$(MAKE) -C candle clean

.PHONY: init clean release
