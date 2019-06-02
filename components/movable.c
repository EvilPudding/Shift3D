#include <candle.h>
#include "movable.h"
#include "level.h"
#include "side.h"
#include "grid.h"
#include <stdlib.h>
#include <components/spatial.h>
#include <components/model.h>
#include "../openal.candle/speaker.h"

c_movable_t *c_movable_new(int value)
{
	c_movable_t *self = component_new("movable");
	self->value = value;
	entity_add_component(c_entity(self), c_speaker_new());

	return self;
}

void push_at(entity_t lvl, int x, int y, int z, int value, vec3_t from)
{
	khiter_t k;
	c_level_t *level = c_level(&lvl);
	c_grid_t *gc = c_grid(&level->grid);

	ct_t *movables = ecm_get(ref("movable"));
	vec3_t pos = vec3(x, y, z);

	vec3_t dif = vec3_sub(from, pos);
	if(fabs(dif.x) > fabs(dif.z))
	{
		if(dif.x < 0) dif.x = -1;
		if(dif.x > 0) dif.x = 1;
		dif.z = 0;
	}
	else
	{
		if(dif.z < 0) dif.z = -1;
		if(dif.z > 0) dif.z = 1;
		dif.x = 0;
	}

	int air = !(value & 1);
	int next = c_grid_get(gc,
			round(x - dif.x), y, round(z - dif.z));

	if((next & 1) != air || next == -1)
	{
		return;
	}
	c_grid_set(gc, x, y, z, air);

	for(k = kh_begin(movables->cs); k != kh_end(movables->cs); ++k)
	{
		if(!kh_exist(movables->cs, k)) continue;
		c_movable_t *m = (c_movable_t*)kh_value(movables->cs, k);
		if(m->value == value) if(vec3_dist(c_spatial(m)->pos, pos) < 0.4)
		{
			m->mx = -dif.x;
			m->my = -dif.y;
			m->mz = -dif.z;
			m->moving = 1;
			m->x = x;
			m->y = y;
			m->z = z;
			c_speaker_t *speaker = c_speaker(m);
			c_speaker_play(speaker, sauces("drag.wav"), 0);
		}
	}
}

static int c_movable_update(c_movable_t *self, float *dt)
{
	if(!self->moving) return CONTINUE;
	float inc = (*dt) * 6;
	c_spatial_t *sc = c_spatial(self);

	if(self->mx > 0)	
	{
		inc = fmin(inc, self->mx);
		sc->pos.x += inc;
		self->mx -= inc;
		c_spatial_set_pos(sc, sc->pos);
	}
	else if(self->mx < 0)	
	{
		inc = fmin(inc, -self->mx);
		sc->pos.x -= inc;
		self->mx += inc;
		c_spatial_set_pos(sc, sc->pos);
	}
	else if(self->mz > 0)	
	{
		inc = fmin(inc, self->mz);
		sc->pos.z += inc;
		self->mz -= inc;
		c_spatial_set_pos(sc, sc->pos);
	}
	else if(self->mz < 0)	
	{
		inc = fmin(inc, -self->mz);
		sc->pos.z -= inc;
		self->mz += inc;
		c_spatial_set_pos(sc, sc->pos);
	}
	else
	{
		c_side_t *ss = c_side(self);
		c_level_t *level = c_level(&ss->level);
		c_grid_t *gc = c_grid(&level->grid);

		int side = ss->side & 1;
		int dir = side ? 1 : -1;
		vec3_t rnd = vec3_round(sc->pos);

		int ground = c_grid_get(gc, rnd.x, rnd.y + dir, rnd.z);

		if((ground&1) != side || ground == -1)
		{
			c_spatial_set_pos(sc, rnd);
			c_grid_set(gc, sc->pos.x, sc->pos.y, sc->pos.z, 2 | !side);
			if(self->sy)
			{
				c_speaker_t *speaker = c_speaker(self);
				c_speaker_play(speaker, sauces("thump.wav"), 0);
			}
			self->moving = 0;
			self->sy = 0;
			if((side&1) != (c_side(&level->pov)->side&1))
			{
				entity_signal(c_entity(self), sig("grid_update"), NULL, NULL);
			}
		}
		else
		{
			self->sy += dir * 0.4 * *dt;
			sc->pos.y += self->sy;
			c_spatial_set_pos(sc, sc->pos);
		}
	}

	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("movable", sizeof(c_movable_t), NULL, NULL, 0);

	ct_listener(ct, WORLD, sig("world_update"), c_movable_update);
}


