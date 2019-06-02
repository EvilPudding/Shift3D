#include <candle.h>
#include <components/spatial.h>
#include <components/node.h>
#include <utils/nk.h>
#include "side.h"
#include "level.h"
#include "grid.h"
#include "character.h"
#include "side_follow.h"
#include <stdlib.h>

c_side_follow_t *c_side_follow_new()
{
	c_side_follow_t *self = component_new("side_follow");
	self->active = 1;
	return self;
}

static int c_side_follow_update(c_side_follow_t *self, float *dt)
{
	if(!self->active) return CONTINUE;
	c_side_t *ss = c_side(self);
	if(!ss) return CONTINUE;
	c_level_t *level = c_level(&ss->level);

	ss = c_side(&level->pov);
	if(!ss) return CONTINUE;

	int side = ss->side & 1;
	c_spatial_t *sc = c_spatial(self);
	vec3_t pos = sc->pos;

	vec3_t dest = vec3(pos.x, 2.0 + 7.0 * !side, pos.z);
	vec3_t inc = vec3_sub(pos, dest);
	float dist = vec3_len(inc);
	if(dist > 0.1)
	{
		pos = vec3_sub(pos, vec3_scale(inc, *dt));
		c_spatial_set_pos(sc, pos);
	}
	
	return CONTINUE;
}

int c_side_follow_menu(c_side_follow_t *self, void *ctx)
{
	self->active = nk_check_label(ctx, "Follow side", self->active);

	return CONTINUE;
}


REG()
{
	ct_t *ct = ct_new("side_follow", sizeof(c_side_follow_t),
			NULL, NULL, 1, ref("spatial"));

	ct_listener(ct, WORLD, sig("world_update"), c_side_follow_update);
	ct_listener(ct, WORLD, sig("component_menu"), c_side_follow_menu);
}

