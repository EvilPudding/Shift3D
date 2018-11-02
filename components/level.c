#include <candle.h>
#include <components/node.h>
#include <components/name.h>
#include <components/model.h>
#include <components/light.h>
#include "components/key.h"
#include "components/movable.h"
#include "components/grid.h"
#include "components/bridge.h"
#include "components/door.h"
#include "character.h"
#include "charlook.h"
#include "mirror.h"
#include "level.h"
#include "side.h"
#include <stdlib.h>
#include <string.h>

c_level_t *c_level_new(const char *filename, int32_t active)
{
	c_level_t *self = component_new("level");

	if(!filename[0]) return self;

	char open_map_name[256];
	sprintf(open_map_name, "resauces/maps/%s.xmap", filename);

	strcpy(self->file, filename);

	candle_run(c_entity(self), open_map_name);

	c_level_set_active(self, active);

	return self;
}

static void activate_node(entity_t e, int32_t active)
{
	uint32_t i;
	uint32_t visible_group;
	uint32_t light_group;
	uint32_t selectable_group;
	uint32_t transparent_group;
	if(active == 0)
	{
		visible_group = 0;
		light_group = 0;
		selectable_group = 0;
		transparent_group = 0;
	}
	else if(active == 1)
	{
		visible_group = ref("visible");
		light_group = ref("light");
		selectable_group = ref("selectable");
		transparent_group = ref("transparent");
	}
	else
	{
		visible_group = ref("next_level");
		light_group = ref("next_level_light");
		selectable_group = 0;
		transparent_group = 0;
	}
	c_model_t *mc = c_model(&e);
	c_grid_t *gc = c_grid(&e);
	c_key_t *kc = c_key(&e);
	c_bridge_t *bc = c_bridge(&e);
	c_light_t *lc = c_light(&e);
	c_door_t *dc = c_door(&e);
	c_node_t *nc = c_node(&e);
	c_side_t *sc = c_side(&e);
	if(mc)
	{
		c_model_set_groups(mc, visible_group, transparent_group,
				selectable_group);
	}
	if(lc)
	{
		c_light_set_groups(lc, visible_group, ref("ambient"), light_group);
		if(active == 1)
		{
			c_light_set_shadow_cooldown(lc, 4);
		}
		else
		{
			c_light_set_shadow_cooldown(lc, 20);
		}
	}
	if(dc) c_door_set_active(dc, active);
	if(gc) gc->active = active;
	if(kc) kc->active = active;
	if(bc) c_bridge_set_active(bc, active);

	if(nc) for(i = 0; i < nc->children_size; i++)
	{
		activate_node(nc->children[i], active);
	}
	if(sc) c_side_changed(sc, &e);
}

void c_level_reset(c_level_t *self)
{
	c_level_set_active(self, 0);

	entity_t lvl = entity_new(c_name_new(self->file), c_level_new(self->file, 0));
	c_level_set_active(c_level(&lvl), 1);

	entity_destroy(c_entity(self));
}

void c_level_set_active(c_level_t *self, int32_t active)
{
	activate_node(c_entity(self), active);
	if(active == 1)
	{
		c_charlook_t *cam = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0);
		c_character_t *fc = (c_character_t*)ct_get_nth(ecm_get(ref("character")), 0);
		c_mirror_t *mir = (c_mirror_t*)ct_get_nth(ecm_get(ref("mirror")), 0);

		self->mirror = c_entity(mir);
		/* self->character = c_entity(fc); */
		self->pov = c_entity(cam);

		c_side(cam)->level = c_side(fc)->level = c_entity(self);

		c_spacial_t *spawn = c_spacial(&self->spawn);
		c_spacial_t *sc = c_spacial(fc);
		if(vec3_len(vec3_sub(sc->pos, spawn->pos)) > 1.0f)
		{
			c_spacial_set_pos(sc, spawn->pos);
		}
	}
}

REG()
{
	ct_new("level", sizeof(c_level_t), NULL, NULL, 1, ref("node"));
}
