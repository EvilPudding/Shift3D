#ifndef CHARLOOK_H
#define CHARLOOK_H

#include <ecs/ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	float win_min_side;
	float sensitivity;
	float xrot;
	float yrot;
	float zrot;
	int side;
	entity_t x;
} c_charlook_t;

DEF_CASTER("charlook", c_charlook, c_charlook_t);

c_charlook_t *c_charlook_new(entity_t x, float sensitivity);

void c_charlook_register(void);

vec3_t c_charlook_up(c_charlook_t *self);
void c_charlook_toggle_side(c_charlook_t *self);

#endif /* !CHARLOOK_H */
