#ifndef PHYSICS_H
#define PHYSICS_H

#include "../candle/utils/glutil.h"
#include "../candle/utils/material.h"
#include "../candle/utils/texture.h"
#include "../candle/utils/mesh.h"
#include "../candle/utils/shader.h"
#include "../candle/ecs/ecm.h"

typedef float(*collider_cb)(c_t *self, vec3_t pos);
typedef float(*velocity_cb)(c_t *self, vec3_t pos);

typedef struct c_physics_t
{
	c_t super;
	/* currently, physics has no options */
} c_physics_t;

DEF_CASTER(ct_physics, c_physics, c_physics_t)

c_physics_t *c_physics_new(void);

#endif /* !PHYSICS_H */
