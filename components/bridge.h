#ifndef BRIDGE_H
#define BRIDGE_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
    int ix1;
    int iy1;
    int iz1;
    int ix2;
    int iy2;
    int iz2;
    int cx;
    int cy;
    int cz;

	vec3_t min;
	vec3_t max;
    int key;

	mat4_t inverse_model;
	vec3_t rotate_to;

} c_bridge_t;

DEF_CASTER("bridge", c_bridge, c_bridge_t);

c_bridge_t *c_bridge_new(void);
void c_bridge_ready(c_bridge_t *self);
void c_bridge_register(void);

#endif /* !BRIDGE_H */
