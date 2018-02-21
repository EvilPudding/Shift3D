#include "moving.h"
#include <stdlib.h>

DEC_CT(ct_moving);

void c_moving_init(c_moving_t *self)
{
    self->active=1;
}

c_moving_t *c_moving_new(float x, float y, float z, float mx, float mz)
{
	c_moving_t *self = component_new(ct_moving);

    self->x=x;
    self->y=y;
    self->z=z;
    self->mx=mx;
    self->mz=mz;

	return self;
}

void c_moving_register()
{
	ct_new("c_moving", &ct_moving, sizeof(c_moving_t),
			(init_cb)c_moving_init, 0);
}


