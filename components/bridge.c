#include "bridge.h"
#include <components/rigid_body.h>
#include <components/spacial.h>
#include <components/model.h>
#include <stdlib.h>
#include <candle.h>


static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos);

c_bridge_t *c_bridge_new()
{
	c_bridge_t *self = component_new("bridge");

	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_bridge_collider));

	return self;
}

void c_bridge_ready(c_bridge_t *self)
{

	mesh_t *mesh = mesh_new();
	mesh_cuboid(mesh, 0.5,
			vec3(self->min.x - 0.005f, self->min.y - 0.005f, self->min.z - 0.005f),
			vec3(self->max.x + 0.005f, self->max.y + 0.005f, self->max.z + 0.005f));

	mat_t *mat = sauces("bridge.mat");
	mat->transparency.color = vec4(0.3f, 0.3f, 0.0f, 1.0);

	entity_add_component(c_entity(self),
			c_model_new(mesh, sauces("bridge.mat"), 1, 1));
}

void c_bridge_set_active(c_bridge_t *self, int active)
{
	/* if(active) */
	/* { */
		/* c_spacial_set_model(c_spacial(self), self->original_model); */
	/* } */
	self->active = active;
}

static int c_bridge_spacial_changed(c_bridge_t *self)
{
	c_spacial_t *spacial = c_spacial(self);
	self->inverse_model = mat4_invert(spacial->model_matrix);

	return CONTINUE;
}

static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_bridge_t *b = c_bridge(self);
	if(b->active != 1) return -1;
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
	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("bridge", sizeof(c_bridge_t), NULL, NULL,
			1, ref("spacial"));

	ct_listener(ct, ENTITY, sig("spacial_changed"), c_bridge_spacial_changed);

	ct_listener(ct, WORLD, sig("world_update"), c_bridge_update);
}

