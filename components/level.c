#include "../candle/candle.h"
#include "../candle/components/node.h"
#include "../candle/components/name.h"
#include "../candle/components/model.h"
#include "../candle/components/light.h"
#include "../candle/systems/editmode.h"
#include "key.h"
#include "movable.h"
#include "grid.h"
#include "bridge.h"
#include "door.h"
#include "character.h"
#include "force.h"
#include "velocity.h"
#include "rigid_body.h"
#include "charlook.h"
#include "mirror.h"
#include "level.h"
#include "side.h"
#include <string.h>

c_level_t *c_level_new(const char *filename, int32_t active)
{
	c_level_t *self = component_new(ct_level);
	char *bytes;
	size_t bytes_num;
	resource_t *sauce;

	if(!filename[0])
	{
		return self;
	}

	char open_map_name[512];
	sprintf(open_map_name, "%.*s.xmap", 128, filename);

	strcpy(self->file, filename);

	sauce = c_sauces_get_sauce(c_sauces(&SYS), sauce_handle(open_map_name));
	if (!sauce)
	{
		return self;
	}

	bytes = c_sauces_get_bytes(c_sauces(&SYS), sauce, &bytes_num);
	if (!bytes)
	{
		return self;
	}

	candle_run_from_memory(c_entity(self), bytes, bytes_num);
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
		c_spatial_set_pos(c_spatial(lc), c_spatial(lc)->pos);
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
			c_light_set_shadow_cooldown(lc, 1);
		}
		else
		{
			c_light_set_shadow_cooldown(lc, 4);
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

	entity_t lvl = entity_new({
		c_name_new(self->file);
		c_level_new(self->file, 0);
	});
	c_level_t *level = c_level(&lvl);
	c_level_set_active(level, 1);

		c_charlook_t *cam = (c_charlook_t*)ct_get_nth(ecm_get(ct_charlook), 0);
		c_character_t *fc = (c_character_t*)ct_get_nth(ecm_get(ct_character), 0);
		c_mirror_t *mir = (c_mirror_t*)ct_get_nth(ecm_get(ct_mirror), 0);

		c_velocity(fc)->velocity = vec3(0.0f, 0.0f, 0.0f);
		fc->max_jump_vel = 0.0f;

		c_side_t *charside = c_side(fc);
		level->mirror = c_entity(mir);
		mir->follow = c_entity(cam);
		/* level->character = c_entity(fc); */
		level->pov = c_entity(cam);

		c_side(cam)->level = c_side(fc)->level = c_entity(level);

		c_spatial_t *spawn = c_spatial(&level->spawn);
		c_side_t *spawnside = c_side(spawn);
		c_spatial_t *sc = c_spatial(fc);
		c_spatial_t *body = c_spatial(&fc->orientation);

		c_spatial_lock(body);
		c_spatial_lock(sc);

		c_spatial_set_model(body, mat4());
		c_spatial_set_model(sc, spawn->model_matrix);
		c_charlook_reset(cam);
		fc->targR = 0;
		int side_dir = (spawnside->side & 1) ? 1 : -1;
		c_rigid_body(fc)->offset =  -0.8f * side_dir;

		c_force(&fc->force_down)->force = vec3(0.0, 23 * side_dir, 0.0);

		charside->side = spawnside->side;
		c_spatial_unlock(body);
		c_spatial_unlock(sc);

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

void ct_level(ct_t *self)
{
	ct_init(self, "level", sizeof(c_level_t));
	ct_add_dependency(self, ct_node);
}
