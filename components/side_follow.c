#include "../candle/candle.h"
#include "../candle/components/spatial.h"
#include "../candle/components/node.h"
#include "../candle/utils/nk.h"
#include "side.h"
#include "level.h"
#include "grid.h"
#include "character.h"
#include "side_follow.h"

c_side_follow_t *c_side_follow_new()
{
	c_side_follow_t *self = component_new(ct_side_follow);
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


void ct_side_follow(ct_t *self)
{
	ct_init(self, "side_follow", sizeof(c_side_follow_t));
	ct_add_dependency(self, ct_node);

	ct_add_listener(self, WORLD, 0, ref("world_update"), c_side_follow_update);
	ct_add_listener(self, WORLD, 0, ref("component_menu"), c_side_follow_menu);
}

