#include "../candle/components/node.h"
#include "../candle/components/model.h"
#include "../candle/components/light.h"
#include "../candle/components/name.h"
#include "../candle/systems/editmode.h"
#include "side.h"
#include "level.h"
#include "grid.h"
#include "charlook.h"

static int c_side_position_changed(c_side_t *self);

void c_side_init(c_side_t *self)
{
	self->side = 0;
}

c_side_t *c_side_new(entity_t level, int side, int locked)
{
	c_side_t *self = component_new(ct_side);
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
		c_node_t *nc = c_node(self);
		if (!c_node(self)) return CONTINUE;
		vec3_t pos = vec3_round(c_node_pos_to_global(nc, vec3(0.0f, 0.0f, 0.0f)));

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

void ct_side(ct_t *self)
{
	ct_init(self, "side", sizeof(c_side_t));
	ct_set_init(self, (init_cb)c_side_init);
	ct_add_dependency(self, ct_node);
	ct_add_listener(self, WORLD,  0, ref("side_changed"), c_side_changed);
	ct_add_listener(self, WORLD,  0, ref("level_changed"), c_side_level_changed);
	ct_add_listener(self, ENTITY, 0, ref("node_changed"), c_side_position_changed);
	ct_add_listener(self, WORLD,  0, ref("component_menu"), c_side_menu);
}


