#include <candle.h>
#include <components/node.h>
#include <components/name.h>
#include "level.h"
#include "side.h"
#include <stdlib.h>
#include <string.h>


void c_level_init(c_level_t *self) { }

c_level_t *c_level_new(candle_t *engine, const char *filename)
{
	c_level_t *self = component_new(ct_level);

	strcpy(self->file, filename);

	if(!filename[0]) return self;

	self->scene = entity_new(c_name_new("scene"), c_node_new());

	candle_run(engine, self->scene, filename);

	self->grid = c_node_get_by_name(c_node(&self->scene), "grid");

	return self;
}

DEC_CT(ct_level)
{
	ct_new("c_level", &ct_level, sizeof(c_level_t),
			(init_cb)c_level_init, 1, ct_side);
}
