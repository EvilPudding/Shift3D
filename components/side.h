#ifndef SIDE_H
#define SIDE_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	int side;
} c_side_t;

extern unsigned long ct_side;

DEF_CASTER(ct_side, c_side, c_side_t)

c_side_t *c_side_new(int side);
c_side_t *c_side(entity_t entity);
void c_side_register(ecm_t *ecm);

#endif /* !SIDE_H */
