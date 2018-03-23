#include "side.h"
#include <stdlib.h>


void c_side_init(c_side_t *self)
{
	self->side = 2;
}

c_side_t *c_side_new(int side)
{
	c_side_t *self = component_new(ct_side);

	self->side = side;
	return self;
}

DEC_CT(ct_side)
{
	ct_new("c_side", &ct_side, sizeof(c_side_t),
			(init_cb)c_side_init, 0);
}


