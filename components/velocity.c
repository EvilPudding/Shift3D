#include "../candle/ecs/ecm.h"
#include "../candle/components/node.h"
#include "velocity.h"

c_velocity_t *c_velocity_new(float x, float y, float z)
{
	c_velocity_t *self = component_new(ct_velocity);

	self->velocity = vec3(x, y, z);

	return self;
}

void c_velocity_set_normal(c_velocity_t *self, vec3_t nor)
{
	self->normal = nor;
}

void c_velocity_set_vel(c_velocity_t *self, float x, float y, float z)
{
	self->velocity.x = x;
	self->velocity.y = y;
	self->velocity.z = z;
}

void ct_velocity(ct_t *self)
{
	ct_init(self, "velocity", sizeof(c_velocity_t));
	ct_add_dependency(self, ct_node);
}

