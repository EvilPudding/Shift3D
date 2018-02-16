#ifndef MOVING_H
#define MOVING_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
    float x,y,z;
    float mx, my, mz;
    float sy;
    int side, active, pushed, XorZ;
} c_moving_t;

DEF_CASTER(ct_moving, c_moving, c_moving_t);

c_moving_t *c_moving_new(float x, float y, float z, float mx, float mz);
void c_moving_register(void);

#endif /* !MOVING_H */
