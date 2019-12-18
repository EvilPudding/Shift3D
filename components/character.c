#include "character.h"
#include "side.h"
#include <candle.h>
#include <components/spatial.h>
#include <components/velocity.h>
#include <components/node.h>
#include <components/rigid_body.h>
#include <components/force.h>
#include <systems/editmode.h>
#include <systems/controller.h>
#include "level.h"
#include "movable.h"
#include "charlook.h"
#include "grid.h"
#include <systems/keyboard.h>
#include <math.h>
#include <stdlib.h>
#include "../openal.candle/speaker.h"

extern int window_width, window_height;

c_character_t *c_character_new(entity_t orientation,
		int plane_movement, entity_t force_down)
{
	c_character_t *self = component_new("character");
	self->plane_movement = plane_movement;
	self->force_down = force_down;

	self->orientation = orientation;

	return self;
}

void c_character_teleport(c_character_t *self, entity_t in, entity_t out)
{
	if(entity_exists(self->in)) return;
	self->in = in;
	self->out = out;
}

static void _c_character_teleport(c_character_t *self)
{
	c_charlook_t *charlook = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0);
	c_spatial_t *cam = c_spatial(charlook);
	c_spatial_t *in = c_spatial(&self->in);
	c_spatial_t *out = c_spatial(&self->out);
	c_spatial_t *body = c_spatial(&self->orientation);
	c_spatial_t *sc = c_spatial(self);
	c_velocity_t *vc = c_velocity(self);
	c_side_t *ss = c_side(self);

	c_side_t *spawn_side = c_side(out);
	int out_side = spawn_side->side & 1;


	mat4_t model = sc->model_matrix;

	vec4_t rot = quat();
	{
		model = mat4_mul(mat4_invert(in->model_matrix), model);
		rot = quat_mul(quat_invert(in->rot_quat), rot);
	}
	{
		model = mat4_mul(out->model_matrix, model);
		rot = quat_mul(out->rot_quat, rot);
	}

	c_level_t *level = c_level(&ss->level);
	c_level_t *next_level = c_level(&spawn_side->level);

	c_spatial_lock(body);
	c_spatial_lock(sc);

	vc->velocity = quat_mul_vec3(rot, vc->velocity);
	if(level != next_level)
	{
		body->rot_quat = quat_mul(body->rot_quat, rot);
		body->modified = 1;
		body->update_id++;
	}

	vec3_t npos = vec4_xyz(mat4_mul_vec4(model, vec4(0.0f, 0.0f, 0.0f, 1.0f)));
	c_spatial_set_pos(sc, npos);

	if(out_side != (ss->side & 1))
	{
		ss->side = !(ss->side & 1);
		c_force_t *force = c_force(&self->force_down);
		force->force = vec3_inv(force->force);

		self->targR = self->targR == 0 ? M_PI : 0;
		c_spatial_rotate_Z(sc, M_PI);

		c_spatial_rotate_Z(body, -M_PI);
		c_spatial_rotate_Y(body, M_PI);
	}

	ss->level = c_side(cam)->level = c_entity(next_level);

	next_level->pov = level->pov;
	next_level->mirror = level->mirror;
	candle_skip_frame(3);

	c_level_set_active(level, 0);
	c_level_set_active(next_level, 1);

	entity_destroy(c_entity(level));

	self->in = entity_null;
	self->out = entity_null;

	c_spatial_unlock(body);
	c_spatial_unlock(sc);

	c_editmode_select(c_editmode(&SYS), c_entity(self));
	c_editmode_select(c_editmode(&SYS), 0);
}


int c_character_update(c_character_t *self, float *dt)
{
	c_side_t *ss = c_side(self);
	if (!ss) return CONTINUE;
	if(self->reset)
	{
		c_editmode_select(c_editmode(&SYS), c_entity(self));
		c_editmode_select(c_editmode(&SYS), 0);
		self->reset = 0;
	}
	c_level_t *level = c_level(&ss->level);

	if(!level || !level->active) return CONTINUE;
	c_grid_t *gc = c_grid(&level->grid);
	if(!gc) return CONTINUE;

	c_spatial_t *ori = c_spatial(&self->orientation);
	float dif;

	c_velocity_t *vc = c_velocity(self);
	vec3_t *vel = &vc->velocity;
	float accel = 67.5f * (*dt);

	if(entity_exists(self->in)) _c_character_teleport(self);

	c_spatial_t *sc = c_spatial(self);
	c_spatial_lock(sc);

	vec3_t front;
	vec3_t sideways;

	const vec3_t gravity = c_force(&self->force_down)->force;
	const vec3_t up = vec3_inv(gravity);
	vec3_t up_dir = vec3_norm(up);
	vec3_t up_line = vec3_mul(up_dir, up_dir);
	vec3_t tang = vec3_norm(vec3_sub(vec3(1.0, 1.0, 1.0), up_line));

	vec4_t ori_rot = quat_mul(sc->rot_quat, ori->rot_quat);
	front = vec3_norm(
		vec3_mul(quat_mul_vec3(ori_rot, vec3(0.0, 0.0, 1.0)), tang));
	sideways = vec3_norm(
		vec3_mul(quat_mul_vec3(ori_rot, vec3(1.0, 0.0, 0.0)), tang));


	int floored = vec3_dot(up_dir, vc->normal) > 0;

	vec3_t next_vel = vec3_add(*vel, vec3_scale(gravity, *dt));
	vec3_t f = vec3_round(vec3_add(sc->pos, vec3_scale(next_vel, *dt)));
	/* vec3_t f = vec3_round(vec3_sub(sc->pos, vec3_scale(up_dir, 0.4))); */
	int bellow_value = c_grid_get(gc, _vec3(f));
	int shiftable = (bellow_value & 1) != (ss->side & 1);

	vec3_t f2 = vec3_round(vec3_add(sc->pos, vec3_scale(up_dir, 0.5f)));
	int bellow_value2 = c_grid_get(gc, _vec3(f2));
	if(bellow_value2 & 4 || self->kill_self) // SPIKES
	{
		self->kill_self = 0;
		c_spatial_unlock(sc);
		c_level_reset(level);

		self->reset = 1;
		return CONTINUE;
	}

	if(self->last_vel.x != 0 || self->last_vel.y != 0 || self->last_vel.z != 0)
	{
		*vel = self->last_vel;
		vec3_t tang_speed = vec3_mul(tang, *vel);
		self->max_jump_vel = fmax(self->max_jump_vel, 5.0f);
		self->max_jump_vel = fmax(vec3_len(tang_speed), self->max_jump_vel);
		self->last_vel = vec3(0.0f, 0.0f, 0.0f);
	}

	vec3_t walk_dir = vec3(0.0f, 0.0f, 0.0f);
	if(self->left)
	{
		walk_dir = vec3_sub(walk_dir, vec3_scale(sideways, self->left));
	}
	if(self->right)
	{
		walk_dir = vec3_add(walk_dir, vec3_scale(sideways, self->right));
	}
	if(self->forward)
	{
		walk_dir = vec3_sub(walk_dir, vec3_scale(front, self->forward));
	}
	if(self->backward)
	{
		walk_dir = vec3_add(walk_dir, vec3_scale(front, self->backward));
	}

	{
		/* clamp dir strength */
		float walk_length = vec3_len(walk_dir);
		if (walk_length > 1.0f)
		{
			walk_dir = vec3_scale(walk_dir, 1.0f / walk_length);
		}
	}

	*vel = vec3_add(*vel, vec3_scale(walk_dir, accel));
	vec3_t tang_speed = vec3_mul(tang, *vel);

	if(self->swap == 1 && shiftable)
	{
		entity_signal(c_entity(self), ref("grid_update"), NULL, NULL);
		self->swap = 2;
		c_force(&self->force_down)->force = up;
		/* c_force(self->force_down)->force = vec3(0.0, 0.0, 30.0); */
		/* *vel = self->last_vel; */

		float oy = sc->pos.y;
		float ox = sc->pos.x;
		float oz = sc->pos.z;
		sc->pos = vec3_round(sc->pos);
		sc->pos = vec3_sub(sc->pos, vec3_scale(up_dir, 0.51));
		if(up_dir.y != 0.0f)
		{
			float dif = oy - sc->pos.y;
			sc->pos.y = oy - dif * 2;
		}
		else if(up_dir.x != 0.0f)
		{
			float dif = ox - sc->pos.x;
			sc->pos.x = ox - dif * 2;
		}
		else if(up_dir.z != 0.0f)
		{
			float dif = oz - sc->pos.z;
			sc->pos.z = oz - dif * 2;
		}

		ss->side = !(ss->side & 1);
		/* c_spatial_set_rot(sc, up_dir.x, up_dir.y, up_dir.z, c_charlook(self->orientation)->yrot); */
		/* c_charlook_toggle_side(c_charlook(&self->orientation)); */
		/* c_spatial_scale(sc, vec3(1, -1, 1)); */
		self->last_vel = vec3_mul(up_line, *vel);

		c_rigid_body(self)->offset = -c_rigid_body(self)->offset;
		c_speaker_play(c_speaker(self), sauces("mag.wav"), 0);

		self->targR = self->targR == 0 ? M_PI : 0;
		goto end;
	}


	if(!floored)
	{
		float tang_len = vec3_len(tang_speed);
		if(tang_len < self->max_jump_vel &&
				!self->forward && !self->right && !self->left)
		{
			self->max_jump_vel = fmax(tang_len, 1.3);
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
			vec3_t t = vec3_add(sc->pos, vec3_scale(vec3_norm(*vel), 0.6));
			t = vec3_round(t);

			int val = c_grid_get(gc, t.x, t.y, t.z);
			if(val & 0x2 && (val & 1) != (ss->side & 1))
			{
				push_at(ss->level, t.x, t.y, t.z, val, sc->pos);
				*vel = vec3(0.0f, 0.0f, 0.0f);
			}
		}
		self->max_jump_vel = fmax(0.8, vec3_len(tang_speed));

		if(self->jump == 1)
		{
			/* self->jump = 2; REPEAT JUMP SAME CLICK? */
			/* self->jump = 0; */
			*vel = vec3_add(*vel, vec3_scale(up_dir, 10));

		}

		/* vec3_t dec = vec3_scale(*vel, 3 * *dt); */
		vec3_t dec = vec3_scale(*vel, 14 * *dt);
		*vel = vec3_sub(*vel, dec);
	}

end:

	c_spatial_unlock(sc);


	sc = c_spatial(&self->orientation);
	if(fabs(dif = self->targR - sc->rot.z) > 0.001)
	{
		float scale = 5.0f * (*dt);
		c_spatial_rotate_Z(sc, dif * scale);
	}

	return CONTINUE;
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
		case 'R': case 'r': self->kill_self = 1; break;
		case '`':
		{
			c_editmode_activate(c_editmode(&SYS));
			break;
		}
		case 32: self->jump = 0; break;
	}
	return CONTINUE;
}

int c_character_key_down(c_character_t *self, char *key)
{
	switch(*key)
	{
		case 'W': case 'w': self->forward = 1.0f; break;
		case 'A': case 'a': self->left = 1.0f; break;
		case 'D': case 'd': self->right = 1.0f; break;
		case 'S': case 's': self->backward = 1.0f; break;
		case 'Q': case 'q': self->swap = self->swap?:1; break;
		case 'E': case 'e': self->pushing = 1; break;
		case 32: self->jump = self->jump?:1; break;
		default: printf("key: %d pressed\n", *key); break;
	}
	return CONTINUE;
}

int c_character_controller(c_character_t *self, controller_axis_t *event)
{
	if (event->side == 0)
	{
		self->backward = fmax( event->y, 0.f);
		self->forward  = fmax(-event->y, 0.f);
		self->left     = fmax(-event->x, 0.f);
		self->right    = fmax( event->x, 0.f);
		return STOP;
	}
	return CONTINUE;
}

int c_character_controller_button_up(c_character_t *self, controller_button_t *event)
{
	if (event->key == SDL_CONTROLLER_BUTTON_A)
	{
		self->jump = 0;
		return STOP;
	}
	if (event->key == SDL_CONTROLLER_BUTTON_X)
	{
		self->pushing = 0;
		return STOP;
	}
	else if (event->key == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
	{
		self->swap = 0;
		return STOP;
	}
	return CONTINUE;
}

int c_character_controller_button_down(c_character_t *self, controller_button_t *event)
{
	if (event->key == SDL_CONTROLLER_BUTTON_A)
	{
		self->jump = self->jump?:1;
		return STOP;
	}
	if (event->key == SDL_CONTROLLER_BUTTON_X)
	{
		self->pushing = 1;
		return STOP;
	}
	else if (event->key == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
	{
		self->swap = self->swap?:1;
		return STOP;
	}
	return CONTINUE;
}


REG()
{
	ct_t *ct = ct_new("character", sizeof(c_character_t),
			NULL, NULL, 4, ref("velocity"), ref("node"),
			ref("rigid_body"), ref("side"));

	ct_listener(ct, WORLD, 0, ref("key_up"), c_character_key_up);

	ct_listener(ct, WORLD, 0, ref("key_down"), c_character_key_down);

	ct_listener(ct, WORLD, 0, ref("world_update"), c_character_update);

	ct_listener(ct, WORLD, 100, ref("controller_axis"), c_character_controller);

	ct_listener(ct, WORLD, 100, ref("controller_button_up"),
	            c_character_controller_button_up);

	ct_listener(ct, WORLD, 100, ref("controller_button_down"),
	            c_character_controller_button_down);
}

