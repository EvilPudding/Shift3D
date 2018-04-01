#ifndef LEVEL_H
#define LEVEL_H

#include <ecm.h>
#include <candle.h>

typedef struct
{
	c_t super; /* extends c_t */

	entity_t grid;
	entity_t scene;
	char file[256];
} c_level_t;

DEF_CASTER("c_level", c_level, c_level_t);

c_level_t *c_level_new(candle_t *engine, const char *filename);
void c_level_register(void);

#endif /* !LEVEL_H */
