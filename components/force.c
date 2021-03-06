#include "force.h"
#include "velocity.h"


void c_force_init(c_force_t *self)
{
	self->force = vec3(0.0, 0.0, 0.0);
	self->active = 1;
}

void ct_force(ct_t *self)
{
	ct_init(self, "force", sizeof(c_force_t));
	ct_set_init(self, (init_cb)c_force_init);
}

c_force_t *c_force_new(float x, float y, float z, int active)
{
	c_force_t *self = component_new(ct_force);

	self->active = active;
	self->force = vec3(x, y, z);

	return self;
}
