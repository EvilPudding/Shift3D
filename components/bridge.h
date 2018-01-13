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
	/* TODO make vec3 */
	float x1,
		  y1,
		  z1;
	/* TODO make vec3 */
	float x2,
		  y2,
		  z2;
    int key;

	/* TODO make vec3 */
	vec3_t pivot;
	vec3_t rotate_to;

} c_bridge_t;

extern unsigned long ct_bridge;

DEF_CASTER(ct_bridge, c_bridge, c_bridge_t);

c_bridge_t *c_bridge_new();
void c_bridge_register(ecm_t *ecm);

#endif /* !BRIDGE_H */
