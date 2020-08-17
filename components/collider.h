#ifndef COLLIDER_H
#define COLLIDER_H

#include "../candle/ecs/ecm.h"
#include "physics.h"

typedef struct
{
	c_t super; /* extends c_t */

	collider_cb cb;
} c_collider_t;

void ct_collider(ct_t *self);
DEF_CASTER(ct_collider, c_collider, c_collider_t)

c_collider_t *c_collider_new(collider_cb cb);

#endif /* !COLLIDER_H */
