#include "door.h"
#include <components/spacial.h>
#include <stdlib.h>
#include <string.h>

DEC_CT(ct_door);

void c_door_init(c_door_t *self)
{
	self->super = component_new(ct_door);

	self->next[0] = '\0';
}

c_door_t *c_door_new(const char *next)
{
	c_door_t *self = malloc(sizeof *self);
	c_door_init(self);

	strcpy(self->next, next);
	return self;
}

void c_door_register(ecm_t *ecm)
{
	ecm_register(ecm, "Door", &ct_door, sizeof(c_door_t),
			(init_cb)c_door_init, 1, ct_spacial);
}

