#ifndef KEY_H
#define KEY_H

#include <ecs/ecm.h>

typedef struct
{
	int id;
} key_activated_data;

typedef struct
{
	c_t super; /* extends c_t */
	vec3_t rot;
	int key;
	uint32_t active;
} c_key_t;

DEF_CASTER(ct_key, c_key, c_key_t);

c_key_t *c_key_new(int rotX, int rotY, int rotZ, int id);

#endif /* !KEY_H */
