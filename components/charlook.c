#include "charlook.h"
#include "side.h"
#include <systems/window.h>
#include <components/spacial.h>
#include <components/force.h>
#include <components/node.h>
#include "level.h"
#include "grid.h"
#include "character.h"
#include <systems/mouse.h>
#include <candle.h>
#include <math.h>
#include <stdlib.h>

static int c_charlook_mouse_move(c_charlook_t *self, mouse_move_data *event);

void c_charlook_init(c_charlook_t *self)
{
	self->win_min_side = 1080;
}

c_charlook_t *c_charlook_new(entity_t x, float sensitivity)
{
	c_charlook_t *self = component_new("charlook");

	self->sensitivity = sensitivity;
	candle_grab_mouse(c_entity(self), 0);

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
	c_spacial_set_model(c_spacial(self), mat4());
	c_spacial_set_pos(c_spacial(self), vec3(0.0, 0.7, 0.0));
	self->xrot = 0;
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

	const float max_up = M_PI / 2.0 - 0.01;
	const float max_down = -M_PI / 2.0 + 0.01;

	c_spacial_t *sc = c_spacial(self);
	if(!sc) return CONTINUE;

	if(self->xrot + inc_y >= max_up && inc_y > 0)
	{
		printf("%f\n", self->xrot);
		inc_y = max_up - self->xrot;
	}
	if(self->xrot + inc_y <= max_down && inc_y < 0)
	{
		printf("%f\n", self->xrot);
		inc_y = max_down - self->xrot;
	}

	c_side_t *sidec = c_side(self);
	int side = sidec->side;
	inc_x = (side & 1) ? -inc_x : inc_x;

	if(inc_y)
	{
		c_spacial_lock(sc);
		self->xrot += inc_y;
		c_spacial_rotate_X(sc, inc_y);
		c_spacial_unlock(sc);
	}

	sc = c_spacial(&self->x);
	if(!sc) return CONTINUE;

	if(inc_x)
	{
		float old_rot = sc->rot.z;
		c_spacial_lock(sc);
		c_spacial_rotate_Z(sc, -old_rot);
		c_spacial_rotate_Y(sc, inc_x);
		c_spacial_rotate_Z(sc, old_rot);
		c_spacial_unlock(sc);
	}
	/* vec4_print(sc->rot_quat); */

	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("charlook", sizeof(c_charlook_t), c_charlook_init, NULL,
			1, ref("node"));

	ct_listener(ct, ENTITY, sig("mouse_move"), c_charlook_mouse_move);

	ct_listener(ct, WORLD, sig("window_resize"), c_charlook_window_resize);
	ct_listener(ct, WORLD, sig("world_update"), c_charlook_update);
}

