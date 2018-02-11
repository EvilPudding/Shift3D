#ifndef KEY_H
#define KEY_H

#include <ecm.h>

typedef struct
{
	int id;
} key_activated_data;

typedef struct
{
	c_t super; /* extends c_t */

	vec3_t rot;
    int key;
} c_key_t;

extern unsigned long ct_key;

DEF_CASTER(ct_key, c_key, c_key_t);

c_key_t *c_key_new(int rotX, int rotY, int rotZ, int id);
void c_key_register(ecm_t *ecm);

#endif /* !KEY_H */
