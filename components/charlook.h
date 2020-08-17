#ifndef CHARLOOK_H
#define CHARLOOK_H

#include "../candle/ecs/ecm.h"

typedef struct
{
	c_t super; /* extends c_t */
	float win_min_side;
	float sensitivity;
	float xrot;
	float yrot;
	float zrot;
	entity_t x;
} c_charlook_t;

void ct_charlook(ct_t *self);
DEF_CASTER(ct_charlook, c_charlook, c_charlook_t);

c_charlook_t *c_charlook_new(entity_t x, float sensitivity);

vec3_t c_charlook_up(c_charlook_t *self);
void c_charlook_reset(c_charlook_t *self);

#endif /* !CHARLOOK_H */
