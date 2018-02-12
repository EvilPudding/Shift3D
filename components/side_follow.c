#include <candle.h>
#include <components/spacial.h>
#include <components/node.h>
#include "side.h"
#include "level.h"
#include "grid.h"
#include "side_follow.h"
#include <stdlib.h>

DEC_CT(ct_side_follow);

void c_side_follow_init(c_side_follow_t *self)
{
	self->super = component_new(ct_side_follow);
}

c_side_follow_t *c_side_follow_new()
{
	c_side_follow_t *self = malloc(sizeof *self);
	c_side_follow_init(self);
	return self;
}

static int c_side_follow_update(c_side_follow_t *self, float *dt)
{
	entity_t entity = c_entity(self);
	int side = c_side(c_ecm(self)->common)->side;
	c_spacial_t *sc = c_spacial(entity);
	vec3_t pos = sc->pos;

	vec3_t dest = vec3(pos.x, 2.0 + 6.0 * !side, pos.z);
	vec3_t inc = vec3_sub(pos, dest);
	float dist = vec3_len(inc);
	if(dist > 0.01)
	{
		pos = vec3_sub(pos, vec3_scale(inc, 5 * *dt));
		c_spacial_set_pos(sc, pos);

		/* TODO: remove string search from this */
		entity_t grid = c_level(c_ecm(self)->common)->grid;
		c_side(entity)->side = c_grid_get(c_grid(grid),
				pos.x, pos.y, pos.z) & 1;
	}
	
	return 1;
}


void c_side_follow_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, "Side Follow", &ct_side_follow,
			sizeof(c_side_follow_t), (init_cb)c_side_follow_init,
			1, ct_spacial);

	ct_register_listener(ct, WORLD, world_update, (signal_cb)c_side_follow_update);
}

