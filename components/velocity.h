#ifndef VELOCITY_H
#define VELOCITY_H

#include <ecs/ecm.h>
#include <utils/glutil.h>

typedef struct
{
	c_t super; /* extends c_t */

	vec3_t pre_movement_pos;
	vec3_t pre_collision_pos;
	vec3_t computed_pos;

	vec3_t normal;
	vec3_t velocity;
} c_velocity_t;

DEF_CASTER("velocity", c_velocity, c_velocity_t)

c_velocity_t *c_velocity_new(float x, float y, float z);
void c_velocity_init(c_velocity_t *self);
void c_velocity_set_vel(c_velocity_t *self, float x, float y, float z);
void c_velocity_set_normal(c_velocity_t *self, vec3_t nor);

#endif /* !VELOCITY_H */