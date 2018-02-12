#include "moving.h"
#include <stdlib.h>

DEC_CT(ct_moving);

void c_moving_init(c_moving_t *self)
{
	self->super = component_new(ct_moving);
    self->sy=0;
    self->active=1;
}

c_moving_t *c_moving_new(float x, float y, float z, float mx, float mz)
{
	c_moving_t *self = malloc(sizeof *self);
	c_moving_init(self);

    self->x=x;
    self->y=y;
    self->z=z;
    self->mx=mx;
    self->mz=mz;

	return self;
}

void c_moving_register(ecm_t *ecm)
{
	ecm_register(ecm, "Moving", &ct_moving, sizeof(c_moving_t),
			(init_cb)c_moving_init, 0);
}


