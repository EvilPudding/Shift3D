#include "side.h"
#include "level.h"
#include "grid.h"
#include "charlook.h"
#include <stdlib.h>
#include <components/node.h>
#include <components/model.h>
#include <components/light.h>
#include <components/name.h>
#include <systems/editmode.h>

static int c_side_position_changed(c_side_t *self);

void c_side_init(c_side_t *self)
{
	self->side = 0;
}

c_side_t *c_side_new(entity_t level, int side, int locked)
{
	c_side_t *self = component_new("side");
	self->level = level;

	self->locked = locked;
	self->side = side;
	c_side_position_changed(self);
	return self;
}

int c_side_menu(c_side_t *self, void *ctx)
{
	int side = nk_check_label(ctx, "side", self->side);
	if(side != self->side)
	{
		self->side = side;
		c_level_t *level = c_level(&self->level);
		if(level)
		{
			entity_signal(c_entity(self), ref("side_changed"),
					&c_entity(self), NULL);
		}
	}
	return CONTINUE;
}

int c_side_changed(c_side_t *self, entity_t *pov)
{
	c_level_t *level = c_level(&self->level);
	if(!level) return CONTINUE;

	if(*pov != c_entity(self) && *pov != level->pov) return CONTINUE;
	int side = c_side(&level->pov)->side;

	int none = side == -1;
	int visible = !none && ((side & 1) == (self->side & 1) || self->side == -1);

	c_model_t *mc = c_model(self);
	if(mc)
	{
		c_model_set_visible(mc, visible);
	}
	c_light_t *lc = c_light(self);
	if(lc)
	{
		c_light_visible(lc, visible);
	}

	return CONTINUE;
}

static int c_side_position_changed(c_side_t *self)
{
	if(self->locked) return CONTINUE;
	if(!entity_exists(self->level)) return CONTINUE;
	c_level_t *level = c_level(&self->level);
	if(!level) return CONTINUE;
	if(!entity_exists(level->pov)) return CONTINUE;

	if(level)
	{
		vec3_t pos = vec3_round(c_node_pos_to_global(c_node(self), vec3(0.0f)));

		c_grid_t *gc = c_grid(&level->grid);
		if(gc)
		{
			int side = c_grid_get(gc, _vec3(pos));
			if(side != self->side)
			{
				self->side = side;
				entity_signal(c_entity(self), ref("side_changed"),
						&c_entity(self), NULL);
			}
		}
	}

	return CONTINUE;
}

static int c_side_level_changed(c_side_t *self, entity_t *level)
{
	if(self->level == *level)
	{
		// activate;
	}
	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("side", sizeof(c_side_t), c_side_init, NULL, 1, ref("node"));
	ct_listener(ct, WORLD, sig("side_changed"), c_side_changed);
	ct_listener(ct, WORLD, sig("level_changed"), c_side_level_changed);
	ct_listener(ct, ENTITY, sig("node_changed"), c_side_position_changed);
	ct_listener(ct, WORLD, sig("component_menu"), c_side_menu);

	signal_init(ref("side_changed"), 0);
	signal_init(ref("level_changed"), 0);
}


