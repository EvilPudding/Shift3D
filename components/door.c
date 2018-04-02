#include "door.h"
#include <components/spacial.h>
#include <stdlib.h>
#include <string.h>


void c_door_init(c_door_t *self) { }

c_door_t *c_door_new(const char *next)
{
	c_door_t *self = component_new("door");

	strcpy(self->next, next);
	return self;
}

REG()
{
	ct_new("door", sizeof(c_door_t), (init_cb)c_door_init, 1,
			ref("spacial"));
}

