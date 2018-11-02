#ifndef LEVEL_H
#define LEVEL_H

#include <ecs/ecm.h>
#include <candle.h>

typedef struct
{
	c_t super; /* extends c_t */

	entity_t grid;
	entity_t spawn;
	entity_t pov;
	entity_t door;
	entity_t mirror;
	char file[256];
	int32_t active;
} c_level_t;

DEF_CASTER("level", c_level, c_level_t);

c_level_t *c_level_new(const char *filename, int32_t active);
void c_level_set_active(c_level_t *self, int32_t active);
void c_level_reset(c_level_t *self);

#endif /* !LEVEL_H */
