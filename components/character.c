#include "character.h"
#include "side.h"
#include <candle.h>
#include <components/spacial.h>
#include <components/velocity.h>
#include <components/node.h>
#include "level.h"
#include "charlook.h"
#include "grid.h"
#include <keyboard.h>
#include <math.h>
#include <stdlib.h>

DEC_CT(ct_character);
extern int window_width, window_height;
int control = 1;

#define friction 0.1

void c_character_init(c_character_t *self)
{
	self->super = component_new(ct_character);

	self->plane_movement = 0;
	self->forward = 0;
	self->backward = 0;
	self->left = 0;
	self->right = 0;
	self->jump = 0;
	self->swap = 0;
	self->targR = 0.0;
	self->max_jump_vel = 0.0;
	self->orientation = entity_null();
	self->force_down = entity_null();
}

c_character_t *c_character_new(entity_t orientation, int plane_movement, entity_t force_down)
{
	c_character_t *self = malloc(sizeof *self);
	c_character_init(self);
	self->plane_movement = plane_movement;
	self->force_down = force_down;

	self->orientation = orientation;

	return self;
}

int c_character_update(c_character_t *self, float *dt)
{
	const float corner = 1.0 / sqrt(2.0);
	c_spacial_t *ori = c_spacial(self->orientation);
	float dif;

	c_velocity_t *vc = c_velocity(c_entity(self));
	vec3_t *vel = &vc->velocity;
	float accel = 72 * (*dt);

	c_spacial_t *sc = c_spacial(self->super.entity);

	vec3_t front;
	vec3_t sideways;

	const vec3_t up = vec3_inv(c_force(self->force_down)->force);
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

	/* front = vec3( */
	/* 		sin(ori->rot.y), */
	/* 		0, */
	/* 		cos(ori->rot.y)); */

	/* sideways = vec3( */
	/* 		cos(-ori->rot.y), */
	/* 		0, */
	/* 		sin(-ori->rot.y)); */


	int floored = vec3_dot(up, vc->normal) > 0;

	if((self->left + self->right) && (self->forward || self->backward))
	{
		accel *= corner;
	}

	front = vec3_scale(front, accel);
	sideways = vec3_scale(sideways, accel);

	if(self->left)
	{
		*vel = vec3_sub(*vel, sideways);
	}

	if(self->right)
	{
		*vel = vec3_add(*vel, sideways);
	}

	if(self->forward)
	{
		*vel = vec3_sub(*vel, front);
	}

	if(self->backward)
	{
		*vel = vec3_add(*vel, front);
	}
	vec3_t tang_speed = vec3_mul(tang, *vel);

	c_level_t *level = c_level(c_ecm(self)->common);
	if(!level)
	{
		floored = 1;
	}

	if(!floored)
	{
		float tang_len = vec3_len(tang_speed);
		if(tang_len > self->max_jump_vel)
		{
			vec3_t norm = vec3_mul(up_line, *vel);
			tang_speed = vec3_scale(tang_speed, self->max_jump_vel / tang_len);
			*vel = vec3_add(tang_speed, norm);
		}
	}
	else
	{
		self->max_jump_vel = fmax(vec3_len(tang_speed), 1);

		if(self->jump == 1)
		{
			/* self->jump = 2; REPEAT JUMP SAME CLICK? */
			/* self->jump = 0; */
			*vel = vec3_add(*vel, vec3_scale(up_dir, 13));

			goto end;
		}

		/* entity_t grid = level->grid; */
		/* if(c_grid_collider(c_grid(grid), vec3_add(sc->pos, up_dir)) < 0) */
		/* { */
		/*	 return 0; */
		/* } */
		if(self->swap == 1)
		{
			self->swap = 2;
			c_force(self->force_down)->force = up;
			/* c_force(self->force_down)->force = vec3(0.0, 0.0, 30.0); */

			c_side(c_ecm(self)->common)->side =
				!c_side(c_ecm(self)->common)->side;
			sc->pos = vec3_round(sc->pos);
			sc->pos = vec3_sub(sc->pos, vec3_scale(up_dir, 0.55));

			/* c_spacial_set_rot(sc, up_dir.x, up_dir.y, up_dir.z, c_charlook(self->orientation)->yrot); */
			if(self->targR == 0)
			{
				self->targR = M_PI;
			}
			else
			{
				self->targR = 0;
			}
			goto end;
		}
		vec3_t dec = vec3_scale(*vel, 11 * *dt);
		*vel = vec3_sub(*vel, dec);
	}
end:

	if(fabs(dif = self->targR - sc->rot.z) > 0.01)
	{
		c_spacial_set_rot(sc, 0, 0, 1, sc->rot.z + dif * 5 * (*dt));
	}

	entity_signal(self->super.entity, spacial_changed, &self->super.entity);
	c_charlook_update(c_charlook(self->orientation));

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
		case 32: self->jump = self->jump?:1; break;
		default: printf("key: %d pressed\n", *key); break;
	}
	return 1;
}

void c_character_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, &ct_character, sizeof(c_character_t),
			(init_cb)c_character_init, 4, ct_spacial, ct_velocity, ct_node,
			ct_rigid_body);

	ct_register_listener(ct, WORLD, key_up, (signal_cb)c_character_key_up);

	ct_register_listener(ct, WORLD, key_down, (signal_cb)c_character_key_down);

	ct_register_listener(ct, WORLD, world_update,
			(signal_cb)c_character_update);
}

