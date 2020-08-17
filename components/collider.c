#include "collider.h"

void ct_collider(ct_t *self)
{
	ct_init(self, "collider", sizeof(c_collider_t));
}

c_collider_t *c_collider_new(collider_cb cb)
{
	c_collider_t *self = component_new(ct_collider);

	self->cb = cb;

	return self;
}
