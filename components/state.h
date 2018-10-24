#ifndef LEVEL_H
#define LEVEL_H

#include <ecs/ecm.h>
#include <candle.h>

typedef struct
{
	c_t super; /* extends c_t */

	entity_t grid;
	entity_t scene;
	entity_t spawn;
	char file[256];
} c_state_t;

DEF_CASTER("state", c_state, c_state_t);

c_state_t *c_state_new(const char *filename);
void c_state_register(void);

#endif /* !LEVEL_H */
