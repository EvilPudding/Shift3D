#ifndef SIDE_H
#define SIDE_H

#include <ecs/ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	int side;
	int locked;
} c_side_t;

DEF_CASTER("side", c_side, c_side_t)

c_side_t *c_side_new(int side, int locked);
void c_side_register(void);

#endif /* !SIDE_H */
