#include "charlook.h"
#include "side.h"
#include <systems/window.h>
#include <components/spatial.h>
#include <components/force.h>
#include <components/node.h>
#include "level.h"
#include "grid.h"
#include "character.h"
#include <systems/mouse.h>
#include <systems/controller.h>
#include <candle.h>
#include <math.h>
#include <stdlib.h>

#define FULL_HEIGHT 1.3
static int c_charlook_mouse_move(c_charlook_t *self, mouse_move_data *event);

void c_charlook_init(c_charlook_t *self)
{
	self->win_min_side = 1080;
}

c_charlook_t *c_charlook_new(entity_t x, float sensitivity)
{
	c_charlook_t *self = component_new("charlook");

	self->sensitivity = sensitivity;
	c_mouse(self)->visible = false;
	c_mouse_activate(c_mouse(self));

	self->x = x;

	return self;
}

static int c_charlook_window_resize(c_charlook_t *self,
		window_resize_data *event)
{
	self->win_min_side = (event->width < event->height) ?
		event->width : event->height;
	return CONTINUE;
}

void c_charlook_reset(c_charlook_t *self)
{
	c_spatial_t *sc = c_spatial(self);
	c_spatial_lock(sc);
	c_spatial_set_model(sc, mat4());
	c_spatial_set_pos(c_spatial(self), vec3(0.0, 1.6, 0.0));
	self->xrot = 0;
	c_spatial_unlock(sc);
}

static int c_charlook_update(c_charlook_t *self)
{
	c_charlook_mouse_move(self, &(mouse_move_data){0});
	return CONTINUE;
}

static int c_charlook_mouse_move(c_charlook_t *self, mouse_move_data *event)
{
	float frac = self->sensitivity / self->win_min_side;
	float inc_x = -event->sx * frac;
	float inc_y = -event->sy * frac;


	c_spatial_t *sc = c_spatial(self);
	if(!sc) return CONTINUE;

	const float max_up = M_PI / 2.0 - 0.01;
	const float max_down = -M_PI / 2.0 + 0.01;
	if(self->xrot + inc_y >= max_up && inc_y > 0)
	{
		inc_y = max_up - self->xrot;
	}
	if(self->xrot + inc_y <= max_down && inc_y < 0)
	{
		inc_y = max_down - self->xrot;
	}

	c_node_t *nc = c_node(self);
	if (!nc) return CONTINUE;
	c_node_t *parent = c_node(&c_node(self)->parent);
	if (!parent) return CONTINUE;
	c_side_t *cside = c_side(ct_get_nth(ecm_get(ref("character")), 0));
	c_grid_t *gc = c_grid(&c_level(&cside->level)->grid);
	vec3_t pos = vec3_round(c_node_pos_to_global(parent, vec3(0.0f, FULL_HEIGHT, 0.0f)));
	int side_above = c_grid_get(gc, _vec3(pos));
	if ((side_above & 1) != (cside->side & 1))
	{
		c_spatial_set_pos(sc, vec3(0.0, 0.8, 0.0));
	}
	else
	{
		c_spatial_set_pos(sc, vec3(0.0, FULL_HEIGHT, 0.0));
	}
	/* c_side_t *side = c_side(self); */
	/* if (!side) return CONTINUE; */
	inc_x = (cside->side & 1) ? -inc_x : inc_x;


	if(inc_y)
	{
		c_spatial_lock(sc);
		self->xrot += inc_y;
		c_spatial_rotate_X(sc, inc_y);
		c_spatial_unlock(sc);
	}

	sc = c_spatial(&self->x);
	if(!sc) return CONTINUE;

	if(inc_x)
	{
		float old_rot = sc->rot.z;
		c_spatial_lock(sc);
		c_spatial_rotate_Z(sc, -old_rot);
		c_spatial_rotate_Y(sc, inc_x);
		c_spatial_rotate_Z(sc, old_rot);
		c_spatial_unlock(sc);
	}

	return CONTINUE;
}

int c_charlook_controller(c_charlook_t *self, controller_axis_t *event)
{
	c_spatial_t *sc = c_spatial(self);
	if (event->side == 1)
	{
		float frac = self->sensitivity / 58.f;

		float inc_y = -(event->y * frac);
		float inc_x = -(event->x * frac);

		const float max_up = M_PI / 2.0 - 0.01;
		const float max_down = -M_PI / 2.0 + 0.01;
		if(self->xrot + inc_y >= max_up && inc_y > 0)
		{
			inc_y = max_up - self->xrot;
		}
		if(self->xrot + inc_y <= max_down && inc_y < 0)
		{
			inc_y = max_down - self->xrot;
		}


	c_node_t *nc = c_node(self);
	if (!nc) return CONTINUE;
	c_node_t *parent = c_node(&c_node(self)->parent);
	if (!parent) return CONTINUE;
	c_side_t *cside = c_side(ct_get_nth(ecm_get(ref("character")), 0));
	c_grid_t *gc = c_grid(&c_level(&cside->level)->grid);
	vec3_t pos = vec3_round(c_node_pos_to_global(parent, vec3(0.0f, FULL_HEIGHT, 0.0f)));
	int side_above = c_grid_get(gc, _vec3(pos));
	if ((side_above & 1) != (cside->side & 1))
	{
		c_spatial_set_pos(sc, vec3(0.0, 0.8, 0.0));
	}
	else
	{
		c_spatial_set_pos(sc, vec3(0.0, FULL_HEIGHT, 0.0));
	}
	/* c_side_t *side = c_side(self); */
	/* if (!side) return CONTINUE; */
	inc_x = (cside->side & 1) ? -inc_x : inc_x;


		if(inc_y)
		{
			c_spatial_lock(sc);
			self->xrot += inc_y;
			c_spatial_rotate_X(sc, inc_y);
			c_spatial_unlock(sc);
		}
		sc = c_spatial(&self->x);
		if(!sc) return CONTINUE;
		if(inc_x)
		{
			float old_rot = sc->rot.z;
			c_spatial_lock(sc);
			c_spatial_rotate_Z(sc, -old_rot);
			c_spatial_rotate_Y(sc, inc_x);
			c_spatial_rotate_Z(sc, old_rot);
			c_spatial_unlock(sc);
		}

		return STOP;
	}
	return CONTINUE;
}


REG()
{
	ct_t *ct = ct_new("charlook", sizeof(c_charlook_t), (init_cb)c_charlook_init, NULL,
			2, ref("node"), ref("mouse"));

	ct_listener(ct, ENTITY, 0, ref("mouse_move"), c_charlook_mouse_move);

	ct_listener(ct, WORLD, 0, ref("window_resize"), c_charlook_window_resize);
	ct_listener(ct, WORLD, 0, ref("world_update"), c_charlook_update);
	ct_listener(ct, WORLD, 100, ref("controller_axis"), c_charlook_controller);
}

