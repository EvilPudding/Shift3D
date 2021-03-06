#ifndef SIDE_H
#define SIDE_H

#include "../candle/ecs/ecm.h"

typedef struct
{
	c_t super; /* extends c_t */
	entity_t level;
	int side;
	int locked;
} c_side_t;

void ct_side(ct_t *self);
DEF_CASTER(ct_side, c_side, c_side_t)

c_side_t *c_side_new(entity_t level, int side, int locked);
int c_side_changed(c_side_t *self, entity_t *pov);

#endif /* !SIDE_H */
