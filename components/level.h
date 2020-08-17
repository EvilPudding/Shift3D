#ifndef LEVEL_H
#define LEVEL_H

#include "../candle/ecs/ecm.h"
#include "../candle/candle.h"

typedef struct
{
	c_t super; /* extends c_t */

	entity_t grid;
	entity_t spawn;
	entity_t pov;
	entity_t door;
	entity_t mirror;
	char file[512];
	int32_t active;
} c_level_t;

void ct_level(ct_t *self);
DEF_CASTER(ct_level, c_level, c_level_t);

c_level_t *c_level_new(const char *filename, int32_t active);
void c_level_set_active(c_level_t *self, int32_t active);
void c_level_reset(c_level_t *self);

#endif /* !LEVEL_H */
