#ifndef SIDE_H
#define SIDE_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	int side;
} c_side_t;

DEF_CASTER("c_side", c_side, c_side_t)

c_side_t *c_side_new(int side);
void c_side_register(void);

#endif /* !SIDE_H */
