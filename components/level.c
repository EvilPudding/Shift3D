#include <candle.h>
#include <components/node.h>
#include <components/name.h>
#include <components/model.h>
#include <components/light.h>
#include <systems/editmode.h>
#include "components/key.h"
#include "components/movable.h"
#include "components/grid.h"
#include "components/bridge.h"
#include "components/door.h"
#include "character.h"
#include "force.h"
#include "velocity.h"
#include "rigid_body.h"
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

static void activate_lights(entity_t e, int32_t active)
{
	uint32_t i;
	c_light_t *lc = c_light(&e);
	c_node_t *nc = c_node(&e);
	if(lc)
	{
		c_spacial_set_pos(c_spacial(lc), c_spacial(lc)->pos);
	}
	if(nc) for(i = 0; i < nc->children_size; i++)
	{
		activate_lights(nc->children[i], active);
	}
}
static void activate_node(entity_t e, int32_t active)
{
	uint32_t i;
	uint32_t visible_group;
	uint32_t shadow_group;
	uint32_t light_group;
	uint32_t selectable_group;
	uint32_t transparent_group;
	if(active == 0)
	{
		visible_group = 0;
		light_group = 0;
		shadow_group = 0;
		selectable_group = 0;
		transparent_group = 0;
	}
	else if(active == 1)
	{
		visible_group = ref("visible");
		shadow_group = ref("shadow");
		light_group = ref("light");
		selectable_group = ref("selectable");
		transparent_group = ref("transparent");
	}
	else
	{
		visible_group = ref("next_level");
		shadow_group = ref("next_level_shadow");
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
		c_model_set_groups(mc, visible_group, shadow_group,
				transparent_group, selectable_group);
	}
	if(lc)
	{
		c_light_set_groups(lc, shadow_group, ref("ambient"), light_group);
		if(active == 1)
		{
			c_light_set_shadow_cooldown(lc, 0);
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
	c_level_t *level = c_level(&lvl);
	c_level_set_active(level, 1);

		c_charlook_t *cam = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0);
		c_character_t *fc = (c_character_t*)ct_get_nth(ecm_get(ref("character")), 0);
		c_mirror_t *mir = (c_mirror_t*)ct_get_nth(ecm_get(ref("mirror")), 0);

		c_velocity(fc)->velocity = vec3(0.0f);
		fc->max_jump_vel = 0.0f;

		c_side_t *charside = c_side(fc);
		level->mirror = c_entity(mir);
		mir->follow = c_entity(cam);
		/* level->character = c_entity(fc); */
		level->pov = c_entity(cam);

		c_side(cam)->level = c_side(fc)->level = c_entity(level);

		c_spacial_t *spawn = c_spacial(&level->spawn);
		c_side_t *spawnside = c_side(spawn);
		c_spacial_t *sc = c_spacial(fc);
		c_spacial_t *body = c_spacial(&fc->orientation);

		c_spacial_lock(body);
		c_spacial_lock(sc);

		c_spacial_set_model(body, mat4());
		c_charlook_reset(cam);
		{
			//c_spacial_set_model(body, mat4());
			//c_spacial_set_model(sc, mat4());
			c_spacial_set_pos(sc, spawn->pos);
			if((charside->side & 1) != (spawnside->side & 1))
			{
				charside->side = spawnside->side;
				c_force_t *force = c_force(&fc->force_down);
				force->force = vec3_inv(force->force);

				c_rigid_body(fc)->offset = -c_rigid_body(fc)->offset;

				fc->targR = fc->targR == 0 ? M_PI : 0;
				/* c_spacial_rotate_Z(sc, M_PI); */

				c_spacial_rotate_Z(body, M_PI);
			}
		}
		c_spacial_unlock(body);
		c_spacial_unlock(sc);

	entity_destroy(c_entity(self));
	entity_signal(level->pov, ref("side_changed"),
			&level->pov, NULL);
}

void c_level_set_active(c_level_t *self, int32_t active)
{
	activate_node(c_entity(self), active);
	activate_lights(c_entity(self), active);
	self->active = active;
}

REG()
{
	ct_new("level", sizeof(c_level_t), NULL, NULL, 1, ref("node"));
}
