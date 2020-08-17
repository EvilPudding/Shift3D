#include "mirror.h"
#include "level.h"
#include "door.h"
#include "side.h"
#include "../candle/components/node.h"
#include "../candle/components/model.h"
#include "../candle/components/light.h"
#include "../candle/components/name.h"
#include "../candle/components/camera.h"

static int c_mirror_update(c_mirror_t *self)
{
	if(!entity_exists(self->follow)) return CONTINUE;

	c_level_t *level = c_level(&c_side(&self->follow)->level);
	entity_t door_in = level->door;
	if(!entity_exists(door_in))
	{
		return CONTINUE;
	}
	c_level_t *next_level = c_level(&c_door(&door_in)->next_level);
	if(!next_level)
	{
		return CONTINUE;
	}

	entity_t door_out = next_level->spawn;
	renderer_t *renderer = c_camera(self)->renderer;

	mat4_t model = renderer->glvars[0].model;

	{
		c_spatial_t *sc2 = c_spatial(&door_in);

		mat4_t m = mat4_invert(sc2->model_matrix);
		model = mat4_mul(m, model);
	}
	if(entity_exists(door_out))
	{
		c_spatial_t *sc2 = c_spatial(&door_out);

		model = mat4_mul(sc2->model_matrix, model);
	}
	else
	{
		printf("no door out\n");
	}

	renderer_set_model(renderer, 1, &model);
	return CONTINUE;
}

void ct_mirror(ct_t *self)
{
	ct_init(self, "mirror", sizeof(c_mirror_t));
	ct_add_dependency(self, ct_node);
	ct_add_listener(self, WORLD, 49, ref("world_pre_draw"), c_mirror_update);
}

c_mirror_t *c_mirror_new(entity_t follow)
{
	c_mirror_t *self = component_new(ct_mirror);

	self->follow = follow;
	return self;
}

