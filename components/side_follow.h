#ifndef SIDE_FOLLOW_H
#define SIDE_FOLLOW_H

#include <ecs/ecm.h>

typedef struct
{
	c_t super; /* extends c_t */
	int active;
} c_side_follow_t;

DEF_CASTER("side_follow", c_side_follow, c_side_follow_t);

c_side_follow_t *c_side_follow_new();
void c_side_follow_register(void);


#endif /* !SIDE_FOLLOW_H */
