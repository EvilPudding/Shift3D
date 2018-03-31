#include "bridge.h"
#include <components/rigid_body.h>
#include <components/spacial.h>
#include <components/model.h>
#include <stdlib.h>
#include <candle.h>


static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos);

void c_bridge_init(c_bridge_t *self)
{
	self->rotate_to = vec3(0.0);
}

c_bridge_t *c_bridge_new()
{
	c_bridge_t *self = component_new(ct_bridge);

	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_bridge_collider));

	return self;
}

void c_bridge_ready(c_bridge_t *self)
{

	mesh_t *mesh = mesh_cuboid(0.5,
			vec3(self->min.x - 0.005f, self->min.y - 0.005f, self->min.z - 0.005f),
			vec3(self->max.x + 0.005f, self->max.y + 0.005f, self->max.z + 0.005f));

	sauces_mat("bridge")->transparency.color = vec4(0.3, 0.3, 0.3, 1.0);

	entity_add_component(c_entity(self),
			c_model_new(mesh, sauces_mat("bridge"), 1));
}

static int c_bridge_spacial_changed(c_bridge_t *self)
{
	c_spacial_t *spacial = c_spacial(self);
	self->inverse_model = mat4_invert(spacial->model_matrix);

	return 1;
}

static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_bridge_t *b = c_bridge(self);
	/* c_spacial_t *b = c_spacial(c_entity(self)); */

	pos = mat4_mul_vec4(b->inverse_model, vec4(_vec3(pos), 1.0f)).xyz;

	/* float inc = 0;//-0.01; */
	int val = pos.x > b->min.x && pos.x < b->max.x
		&& pos.y > b->min.y && pos.y < b->max.y
		&& pos.z > b->min.z && pos.z < b->max.z;
	return val ? 4 : -1;
}

static int c_bridge_update(c_bridge_t *self, float *dt)
{
	if(!vec3_null(self->rotate_to))
	{
		c_spacial_t *s = c_spacial(self);
		c_spacial_lock(s);
		vec3_t inc;
		if(fabs(self->rotate_to.x) < 0.01 &&
				fabs(self->rotate_to.y) < 0.01 &&
				fabs(self->rotate_to.z) < 0.01)
		{
			inc = self->rotate_to;
		}
		else
		{
			inc = vec3_scale(self->rotate_to, (*dt) * 3.0f);
		}
		c_spacial_rotate_X(s, inc.x);
		c_spacial_rotate_Y(s, inc.y);
		c_spacial_rotate_Z(s, inc.z);
		c_spacial_unlock(s);

		self->rotate_to = vec3_sub(self->rotate_to, inc);
	}
	return 1;
}

DEC_CT(ct_bridge)
{
	ct_t *ct = ct_new("c_bridge", &ct_bridge,
			sizeof(c_bridge_t), (init_cb)c_bridge_init, 1, ct_spacial);

	ct_listener(ct, ENTITY, sig("spacial_changed"), c_bridge_spacial_changed);

	ct_listener(ct, WORLD, sig("world_update"), c_bridge_update);
}

