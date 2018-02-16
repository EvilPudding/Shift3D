#include "side.h"
#include <stdlib.h>

DEC_CT(ct_side);

void c_side_init(c_side_t *self)
{
	self->super = component_new(ct_side);
	self->side = 2;
}

c_side_t *c_side_new(int side)
{
	c_side_t *self = malloc(sizeof *self);
	c_side_init(self);

	self->side = side;
	return self;
}

void c_side_register()
{
	ecm_register("Side", &ct_side, sizeof(c_side_t),
			(init_cb)c_side_init, 0);
}


