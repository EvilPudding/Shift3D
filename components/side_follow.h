#ifndef SIDE_FOLLOW_H
#define SIDE_FOLLOW_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
} c_side_follow_t;

extern unsigned long ct_side_follow;

DEF_CASTER(ct_side_follow, c_side_follow, c_side_follow_t);

c_side_follow_t *c_side_follow_new();
void c_side_follow_register(ecm_t *ecm);


#endif /* !SIDE_FOLLOW_H */
