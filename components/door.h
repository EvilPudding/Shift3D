#ifndef DOOR_H
#define DOOR_H

#include <ecs/ecm.h>
#include <utils/renderer.h>
#include <utils/drawable.h>

typedef struct
{
	c_t super; /* extends c_t */

    int activated;
	char next[256];
	drawable_t draw;
	renderer_t *renderer;
	entity_t mirror;
	entity_t next_level;
	int32_t active;
	int modified;
} c_door_t;

DEF_CASTER("door", c_door, c_door_t);

c_door_t *c_door_new(const char *next);
void c_door_set_active(c_door_t *self, int active);

#endif /* !DOOR_H */
