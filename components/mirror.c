#include "mirror.h"
#include <stdlib.h>
#include <components/node.h>
#include <components/model.h>
#include <components/light.h>
#include <components/name.h>
#include <components/camera.h>

c_mirror_t *c_mirror_new(entity_t follow, entity_t door_in, entity_t door_out)
{
	c_mirror_t *self = component_new("mirror");

	self->follow = follow;
	self->door_in = door_in;
	self->door_out = door_out;
	return self;
}

static int c_mirror_update(c_mirror_t *self)
{
	if(entity_exists(self->follow))
	{
		c_node_t *node = c_node(&self->follow);
		c_node_update_model(node);
		mat4_t model = node->model;
		vec4_t rot = node->rot;

		if(entity_exists(self->door_in))
		{
			c_spacial_t *sc2 = c_spacial(&self->door_in);

			mat4_t m = mat4_invert(sc2->model_matrix);
			model = mat4_mul(m, model);
			rot = quat_mul(quat_invert(sc2->rot_quat), rot);
		}
		if(entity_exists(self->door_out))
		{
			/* printf("here\n"); */
			c_spacial_t *sc2 = c_spacial(&self->door_out);

			model = mat4_mul(sc2->model_matrix, model);
		}

		vec3_t pos = mat4_mul_vec4(model, vec4(0.0f, 0.0f, 0.0f, 1.0f)).xyz;
		c_spacial_t *sc = c_spacial(self);
		c_spacial_lock(sc);

		sc->rot_quat = rot;
		sc->pos = pos;

		sc->modified = 1;
		sc->update_id++;
		c_spacial_unlock(sc);
		c_camera_pre_draw(c_camera(self));
	}
	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("mirror", sizeof(c_mirror_t), NULL, NULL, 1, ref("node"));
	ct_listener(ct, WORLD | 49, sig("world_pre_draw"), c_mirror_update);
}


