#include "charlook.h"
#include "side.h"
#include <systems/window.h>
#include <components/spacial.h>
#include <components/force.h>
#include <components/node.h>
#include <mouse.h>
#include <math.h>
#include <stdlib.h>

unsigned long ct_charlook;

void c_charlook_init(c_charlook_t *self)
{
	self->super = component_new(ct_charlook);
	self->win_min_side = 1080;

	self->xrot = 0;
	self->yrot = 0;
	self->x_control = entity_null();
	self->y_control = entity_null();
	self->force_down = entity_null();
}

c_charlook_t *c_charlook_new(entity_t force_down, float sensitivity)
{
	c_charlook_t *self = malloc(sizeof *self);
	c_charlook_init(self);

	self->sensitivity = sensitivity;
	self->force_down = force_down;

	return self;
}

void c_charlook_set_controls(c_charlook_t *self,
		entity_t x_control, entity_t y_control)
{
	self->x_control = x_control;
	self->y_control = y_control;
}

static int c_charlook_window_resize(c_charlook_t *self,
		window_resize_data *event)
{
	self->win_min_side = (event->width < event->height) ?
		event->width : event->height;
	return 1;
}

void c_charlook_update(c_charlook_t *self)
{
	const float max_up = M_PI / 2.0 - 0.01;
	const float max_down = -M_PI / 2.0 + 0.01;

	if(entity_is_null(self->x_control) || entity_is_null(self->y_control))
	{
		return;
	}

	c_spacial_t *sc_x = c_spacial(self->x_control);
	c_spacial_t *sc_y = c_spacial(self->y_control);

	if(self->xrot > max_up) self->xrot = max_up;
	if(self->xrot < max_down) self->xrot = max_down;

	c_spacial_t *sc_z = c_spacial(c_node(self->y_control)->parent);
	float r = mat4_mul_vec4(sc_z->rotation_matrix, vec4(0, self->xrot, 0, 1.0)).y;

	mat4_identity(sc_y->rotation_matrix);
	mat4_identity(sc_x->rotation_matrix);

	mat4_rotate(sc_y->rotation_matrix, sc_y->rotation_matrix, 0, 1, 0,
			self->yrot);

	mat4_rotate(sc_x->rotation_matrix, sc_x->rotation_matrix, 1, 0, 0,
		r);

	c_spacial_update_model_matrix(sc_x);
	c_spacial_update_model_matrix(sc_y);
}

int c_charlook_mouse_move(c_charlook_t *self, mouse_move_data *event)
{
	float frac = self->sensitivity / self->win_min_side;

	c_spacial_t *sc_z = c_spacial(c_node(self->y_control)->parent);
	float r = mat4_mul_vec4(sc_z->rotation_matrix, vec4(0, 1, 0, 1.0)).y;

	self->yrot = self->yrot - event->sx * frac;
	self->xrot = self->xrot - event->sy * frac * r;

	c_charlook_update(self);

	return 1;
}

void c_charlook_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, &ct_charlook, sizeof(c_charlook_t),
			(init_cb)c_charlook_init, 2, ct_spacial, ct_node);

	ct_register_listener(ct, WORLD, mouse_move,
			(signal_cb)c_charlook_mouse_move);

	ct_register_listener(ct, SAME_ENTITY, entity_created,
			(signal_cb)c_charlook_update);

	ct_register_listener(ct, WORLD, window_resize,
			(signal_cb)c_charlook_window_resize);
}

