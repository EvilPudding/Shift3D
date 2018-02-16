#include <candle.h>
#include <components/node.h>
#include <components/name.h>
#include "level.h"
#include "side.h"
#include <stdlib.h>
#include <string.h>

DEC_CT(ct_level);

void c_level_init(c_level_t *self)
{
	self->super = component_new(ct_level);
}

c_level_t *c_level_new(candle_t *engine, const char *filename)
{
	c_level_t *self = malloc(sizeof *self);
	c_level_init(self);

	strcpy(self->file, filename);

	if(!filename[0]) return self;

	self->scene = entity_new(c_name_new("scene"), c_node_new());

	candle_import(engine, self->scene, filename);

	self->grid = c_node_get_by_name(c_node(&self->scene), "grid");

	return self;
}

void c_level_register()
{
	ecm_register("Level", &ct_level, sizeof(c_level_t),
			(init_cb)c_level_init, 1, ct_side);
}
