#include "door.h"
#include <components/spacial.h>
#include <stdlib.h>
#include <string.h>

c_door_t *c_door_new(const char *next)
{
	c_door_t *self = component_new("door");

	strcpy(self->next, next);
	return self;
}

REG()
{
	ct_new("door", sizeof(c_door_t), NULL, NULL, 1, ref("spacial"));
}

