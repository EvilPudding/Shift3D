#ifndef DOOR_H
#define DOOR_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */

    int activated;
	char next[256];
} c_door_t;

DEF_CASTER(ct_door, c_door, c_door_t);

c_door_t *c_door_new(const char *next);
void c_door_register(ecm_t *ecm);

#endif /* !DOOR_H */
