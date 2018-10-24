CC = cc
LD = cc

PLUGINS = $(wildcard *.candle) candle

DIR = build

LIBS = -llua

SRCS = $(wildcard *.c) $(wildcard components/*.c)

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS))

PLUGINS_REL = $(patsubst %, %/build/export.a, $(PLUGINS))
PLUGINS_DEB = $(patsubst %, %/build/export_debug.a, $(PLUGINS))

LIBS_REL = $(LIBS) $(PLUGINS_REL)
LIBS_DEB = $(LIBS) $(PLUGINS_DEB)

CFLAGS = -Wall -I. -Icandle -DUSE_VAO \
		 $(shell sdl2-config --cflags)

CFLAGS_REL = $(CFLAGS) -O3

CFLAGS_DEB = $(CFLAGS) -g3


all: init $(DIR)/shift
	cp -rvu resauces $(DIR)

$(DIR)/shift: $(OBJS_REL) $(PLUGINS_REL)
	$(LD) -o $@ $(OBJS_REL) $(LIBS_REL) $(shell cat $(DIR)/deps)

%/build/export.a:
	$(MAKE) -C $(patsubst %/build/export.a, %, $@)
	echo " " >> $(DIR)/deps
	-cat $(patsubst %/build/export.a, %/build/deps, $@) >> $(DIR)/deps

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

##############################################################################

debug: init $(DIR)/shift_debug
	rm $(PLUGINS_DEB)
	cp -rvu resauces $(DIR)

$(DIR)/shift_debug: $(OBJS_DEB) $(PLUGINS_DEB)
	$(LD) -o $@ $(OBJS_DEB) $(LIBS_DEB) $(shell cat $(DIR)/deps)

%/build/export_debug.a:
	$(MAKE) -C $(patsubst %/build/export_debug.a, %, $@) debug
	echo " " >> $(DIR)/deps
	-cat $(patsubst %/build/export_debug.a, %/build/deps, $@) >> $(DIR)/deps

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components
	rm -f $(DIR)/deps
	touch $(DIR)/deps
	rm -f $(PLUGINS_REL)
	rm -f $(PLUGINS_DEB)

##############################################################################

run: all
	cp -rvu resauces $(DIR)
	$(DIR)/shift 0

gdb: debug
	cp -rvu resauces $(DIR)
	gdb $(DIR)/shift_debug

valgrind: debug
	cp -rvu resauces $(DIR)
	valgrind --log-fd=1 --suppressions=val_sup $(DIR)/shift_debug 10 | tee val_log | less
		
clean:
	rm -r $(DIR)
	$(MAKE) -C candle clean
