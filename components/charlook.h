#ifndef CHARLOOK_H
#define CHARLOOK_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	float win_min_side;
	float sensitivity;
	float xrot;
	float yrot;
	entity_t x_control, y_control, force_down;
} c_charlook_t;

extern unsigned long ct_charlook;

DEF_CASTER(ct_charlook, c_charlook, c_charlook_t);

c_charlook_t *c_charlook_new(entity_t force_down, float sensitivity);

void c_charlook_update(c_charlook_t *self);

void c_charlook_set_controls(c_charlook_t *self,
		entity_t x_control, entity_t y_control);

void c_charlook_register(ecm_t *ecm);

vec3_t c_charlook_up(c_charlook_t *self);

#endif /* !CHARLOOK_H */
