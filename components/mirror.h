#ifndef MIRROR_H
#define MIRROR_H

#include <ecs/ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	entity_t follow;
	entity_t door_in;
	entity_t door_out;
} c_mirror_t;

DEF_CASTER("mirror", c_mirror, c_mirror_t)

c_mirror_t *c_mirror_new(entity_t follow, entity_t door_in, entity_t door_out);

#endif /* !MIRROR_H */
