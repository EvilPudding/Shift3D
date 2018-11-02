#include "mirror.h"
#include "level.h"
#include "door.h"
#include "side.h"
#include <stdlib.h>
#include <components/node.h>
#include <components/model.h>
#include <components/light.h>
#include <components/name.h>
#include <components/camera.h>

c_mirror_t *c_mirror_new(entity_t follow)
{
	c_mirror_t *self = component_new("mirror");

	self->follow = follow;
	return self;
}

static int c_mirror_update(c_mirror_t *self)
{
	if(!entity_exists(self->follow)) return CONTINUE;

	c_level_t *level = c_level(&c_side(&self->follow)->level);
	entity_t door_in = level->door;
	if(!entity_exists(door_in)) return CONTINUE;
	c_level_t *next_level = c_level(&c_door(&door_in)->next_level);
	if(!next_level) return CONTINUE;

	entity_t door_out = next_level->spawn;

	mat4_t model = c_camera(&self->follow)->renderer->glvars[0].model;

	if(entity_exists(door_in))
	{
		c_spacial_t *sc2 = c_spacial(&door_in);

		mat4_t m = mat4_invert(sc2->model_matrix);
		model = mat4_mul(m, model);
	}
	if(entity_exists(door_out))
	{
		c_spacial_t *sc2 = c_spacial(&door_out);

		model = mat4_mul(sc2->model_matrix, model);
	}

	renderer_set_model(c_camera(self)->renderer, c_camera(self)->camid,
			&model);
	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("mirror", sizeof(c_mirror_t), NULL, NULL, 1, ref("node"));
	ct_listener(ct, WORLD | 49, sig("world_pre_draw"), c_mirror_update);
}


