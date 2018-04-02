#include "character.h"
#include "side.h"
#include <candle.h>
#include <components/spacial.h>
#include <components/velocity.h>
#include <components/node.h>
#include <components/rigid_body.h>
#include <components/force.h>
#include "level.h"
#include "movable.h"
#include "charlook.h"
#include "grid.h"
#include <keyboard.h>
#include <math.h>
#include <stdlib.h>

extern int window_width, window_height;
int control = 1;

void c_character_init(c_character_t *self) { }

c_character_t *c_character_new(entity_t orientation, int plane_movement, entity_t force_down)
{
	c_character_t *self = component_new("character");
	self->plane_movement = plane_movement;
	self->force_down = force_down;

	self->orientation = orientation;

	c_rigid_body(self)->offset = 0.8;

	return self;
}

int c_character_update(c_character_t *self, float *dt)
{
	c_level_t *level = c_level(&candle->systems);
	if(!level) return 1;
	c_side_t *ss = c_side(&candle->systems);
	c_grid_t *gc = c_grid(&level->grid);
	if(!gc) return 1;
	const float corner = 1.0f / sqrtf(2.0f);
	c_spacial_t *ori = c_spacial(&self->orientation);
	float dif;

	c_velocity_t *vc = c_velocity(self);
	vec3_t *vel = &vc->velocity;
	float accel = 87.0f * (*dt);

	c_spacial_t *sc = c_spacial(self);

	vec3_t front;
	vec3_t sideways;

	const vec3_t up = vec3_inv(c_force(&self->force_down)->force);
	vec3_t up_dir = vec3_norm(up);
	vec3_t up_line = vec3_mul(up_dir, up_dir);
	vec3_t tang = vec3_norm(vec3_sub(vec3(1.0, 1.0, 1.0), up_line));

	mat4_t ori_rot = mat4_mul(sc->rot_matrix, ori->rot_matrix);
	front = vec3_norm(
		vec3_mul(
			mat4_mul_vec4( ori_rot, vec4(0.0, 0.0, 1.0, 1.0)).xyz,
			tang
		)
	);
	sideways = vec3_norm(
		vec3_mul(
			mat4_mul_vec4( ori_rot, vec4(1.0, 0.0, 0.0, 1.0)).xyz,
			tang
		)
	);


	int floored = vec3_dot(up_dir, vc->normal) > 0;

	vec3_t f = vec3_round(vec3_sub(sc->pos, vec3_scale(up_dir, 0.4)));
	int shiftable = (c_grid_get(gc, f.x, f.y, f.z) & 1) != ss->side;

	if((self->left + self->right) && (self->forward || self->backward))
	{
		accel *= corner;
	}

	if(self->left)
	{
		*vel = vec3_sub(*vel, vec3_scale(sideways, accel));
	}

	if(self->right)
	{
		*vel = vec3_add(*vel, vec3_scale(sideways, accel));
	}

	if(self->forward)
	{
		*vel = vec3_sub(*vel, vec3_scale(front, accel));
	}

	if(self->backward)
	{
		*vel = vec3_add(*vel, vec3_scale(front, accel));
	}
	vec3_t tang_speed = vec3_mul(tang, *vel);


	if(self->swap == 1 && shiftable)
	{
		entity_signal(c_entity(self), sig("grid_update"), NULL);
		self->swap = 2;
		c_force(&self->force_down)->force = up;
		/* c_force(self->force_down)->force = vec3(0.0, 0.0, 30.0); */
		/* *vel = self->last_vel; */

		ss->side = !ss->side;

		sc->pos = vec3_round(sc->pos);
		sc->pos = vec3_sub(sc->pos, vec3_scale(up_dir, 0.55));

		/* c_spacial_set_rot(sc, up_dir.x, up_dir.y, up_dir.z, c_charlook(self->orientation)->yrot); */
		/* c_charlook_toggle_side(c_charlook(&self->orientation)); */
		/* c_spacial_scale(sc, vec3(1, -1, 1)); */

		if(self->targR == 0)
		{
			c_rigid_body(self)->offset = -0.8;
			self->targR = M_PI;
		}
		else
		{
			c_rigid_body(self)->offset = 0.8;
			self->targR = 0;
		}
		goto end;
	}


	if(!floored)
	{
		float tang_len = vec3_len(tang_speed);
		if(tang_len < self->max_jump_vel && !self->forward)
		{
			self->max_jump_vel = fmax(tang_len, 0.3);
		}
		else if(tang_len > self->max_jump_vel)
		{
			vec3_t norm = vec3_mul(up_line, *vel);
			tang_speed = vec3_scale(tang_speed, self->max_jump_vel / tang_len);

			*vel = vec3_add(tang_speed, norm);
		}
	}
	else
	{
		if(self->pushing)
		{
			vec3_t t = vec3_add(sc->pos, vec3_scale(vec3_norm(*vel), 0.45));
			t = vec3_round(t);

			int val = c_grid_get(gc, t.x, t.y, t.z);
			if(val & 0x2)
			{
				push_at(t.x, t.y, t.z, val, sc->pos);
				*vel = vec3(0);
			}
		}

		if(self->jump == 1)
		{
			/* self->jump = 2; REPEAT JUMP SAME CLICK? */
			/* self->jump = 0; */
			*vel = vec3_add(*vel, vec3_scale(up_dir, 10));

			self->max_jump_vel = fmax(0.8, vec3_len(tang_speed));
		}

		vec3_t dec = vec3_scale(*vel, 14 * *dt);
		*vel = vec3_sub(*vel, dec);
	}

end:
	self->last_vel = *vel;




	sc = c_spacial(&self->orientation);
	if(fabs(dif = self->targR - sc->rot.z) > 0.01)
	{
		c_spacial_rotate_Z(sc, dif * 5 * (*dt));
	}

	entity_signal(c_entity(self), sig("spacial_changed"), &c_entity(self));

	return 1;
}

int c_character_key_up(c_character_t *self, char *key)
{
	switch(*key)
	{
		case 'W': case 'w': self->forward = 0; break;
		case 'A': case 'a': self->left = 0; break;
		case 'D': case 'd': self->right = 0; break;
		case 'S': case 's': self->backward = 0; break;
		case 'Q': case 'q': self->swap = 0; break;
		case 'E': case 'e': self->pushing = 0; break;
		case '`': control = !control; break;
		case 32: self->jump = 0; break;
	}
	return 1;
}

int c_character_key_down(c_character_t *self, char *key)
{
	switch(*key)
	{
		case 'W': case 'w': self->forward = 1; break;
		case 'A': case 'a': self->left = 1; break;
		case 'D': case 'd': self->right = 1; break;
		case 'S': case 's': self->backward = 1; break;
		case 'Q': case 'q': self->swap = self->swap?:1; break;
		case 'E': case 'e': self->pushing = 1; break;
		case 32: self->jump = self->jump?:1; break;
		default: printf("key: %d pressed\n", *key); break;
	}
	return 1;
}

REG()
{
	ct_t *ct = ct_new("character", sizeof(c_character_t),
			(init_cb)c_character_init, 3, ref("velocity"), ref("node"),
			ref("rigid_body"));

	ct_listener(ct, WORLD, sig("key_up"), c_character_key_up);

	ct_listener(ct, WORLD, sig("key_down"), c_character_key_down);

	ct_listener(ct, WORLD, sig("world_update"), c_character_update);
}

