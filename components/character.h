#ifndef CHARACTER_H
#define CHARACTER_H

#include <ecs/ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	int forward, backward, left, right, jump, swap, pushing;
	int control;
	float max_jump_vel;

	entity_t orientation;

	vec3_t up;

	int plane_movement;

	entity_t force_down;

	float targR;

	vec3_t last_vel;

	/* TELEPORT */
	entity_t in;
	entity_t out;
} c_character_t;

DEF_CASTER("character", c_character, c_character_t);

c_character_t *c_character_new(entity_t orientation,
		int plane_movement, entity_t force_down);
void c_character_teleport(c_character_t *self, entity_t in, entity_t out);
void c_character_register(void);

#endif /* !CHARACTER_H */
