#ifndef MOVABLE_H
#define MOVABLE_H

#include "../candle/ecs/ecm.h"

typedef struct
{
	c_t super; /* extends c_t */
    float mx, my, mz;
    float sy;
    int moving;

	int x, y, z;
	int value;
} c_movable_t;

DEF_CASTER(ct_movable, c_movable, c_movable_t);

c_movable_t *c_movable_new(int value);

void push_at(entity_t lvl, int x, int y, int z, int value, vec3_t from);

#endif /* !MOVABLE_H */
