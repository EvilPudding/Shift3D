#ifndef MIRROR_H
#define MIRROR_H

#include "../candle/ecs/ecm.h"

typedef struct
{
	c_t super; /* extends c_t */
	entity_t follow;
} c_mirror_t;

void ct_mirror(ct_t *self);
DEF_CASTER(ct_mirror, c_mirror, c_mirror_t)

c_mirror_t *c_mirror_new(entity_t follow);

#endif /* !MIRROR_H */
