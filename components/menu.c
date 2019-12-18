#include "menu.h"
#include <components/name.h>
#include <components/node.h>
#include <components/camera.h>
#include <components/axis.h>
#include <components/attach.h>
#include <components/model.h>
#include <components/charlook.h>
#include <systems/window.h>
#include <systems/keyboard.h>
#include <candle.h>
#include <utils/mesh.h>
#include <vil/vil.h>
#include <stdlib.h>
#include <utils/renderer.h>
#include <utils/nk.h>

static int32_t c_menu_activate_loader(c_menu_t *self);

#define TRANSLATE	0
#define ROTATE		1
#define SCALE		2
#define POLYPEN		3

static renderer_t *menu_renderer_new(c_menu_t *self)
{
	renderer_t *renderer = renderer_new(1.00f);

	texture_t *final =	texture_new_2D(0, 0, 0, 1,
		buffer_new("color",	true, 4)
	);

	renderer_add_tex(renderer, "final", 1.0f, final);

	renderer->output = final;

	renderer->ready = 0;

	return renderer;
}

/* entity_t p; */
c_menu_t *c_menu_new()
{
	c_menu_t *self = component_new("menu");
	return self;
}

void c_menu_init(c_menu_t *self)
{
}

static int32_t c_menu_activate_loader(c_menu_t *self)
{
	void *ctx = nk_can_init(c_window(&SYS)->window); 
	struct nk_font_atlas *atlas; 
	nk_can_font_stash_begin(&atlas); 

	struct nk_font *font0, *font1;
	font0 = nk_font_atlas_add_from_file(atlas, "resauces/Kollektif.ttf", 40.0f, NULL);
	font1 = nk_font_atlas_add_from_file(atlas, "resauces/Kollektif.ttf", 20.0f, NULL);

	nk_can_font_stash_end(); 


	self->nk = ctx;
	self->font0 = font0;
	self->font1 = font1;
	return CONTINUE;
}

int32_t c_menu_mouse_move(c_menu_t *self, mouse_move_data *event)
{
	if (!c_mouse_active(c_mouse(self))) return CONTINUE;

	if(self->control)
	{
		struct nk_context *ctx = self->nk;
		if (ctx)
		{
			if (ctx->input.mouse.grabbed)
			{
				int x = (int)ctx->input.mouse.prev.x, y = (int)ctx->input.mouse.prev.y;
				nk_input_motion(ctx, x + event->sx, y + event->sy);
				return STOP;
			}
			else
			{
				nk_input_motion(ctx, event->x, event->y);
			}
			if (nk_window_is_any_hovered(ctx))
			{
				return STOP;
			}
		}
	}
	return CONTINUE;
}

int32_t c_menu_mouse_press(c_menu_t *self, mouse_button_data *event)
{
	if (!c_mouse_active(c_mouse(self))) return CONTINUE;
	if (nk_window_is_any_hovered(self->nk))
	{
		if (event->button == SDL_BUTTON_LEFT)
		{
			if (event->clicks > 1)
			{
				nk_input_button(self->nk, NK_BUTTON_DOUBLE, event->x, event->y, true);
			}
			nk_input_button(self->nk, NK_BUTTON_LEFT, event->x, event->y, true);
		}
		else if (event->button == SDL_BUTTON_MIDDLE)
		{
			nk_input_button(self->nk, NK_BUTTON_MIDDLE, event->x, event->y, true);
		}
		else if (event->button == SDL_BUTTON_RIGHT)
		{
			nk_input_button(self->nk, NK_BUTTON_RIGHT, event->x, event->y, true);
		}
		return STOP;
	}

	return CONTINUE;
}

int32_t c_menu_mouse_release(c_menu_t *self, mouse_button_data *event)
{
	if (!c_mouse_active(c_mouse(self))) return CONTINUE;
	if(nk_window_is_any_hovered(self->nk) || nk_item_is_any_active(self->nk))
	{
		if (event->button == SDL_BUTTON_LEFT)
		{
			if (event->clicks > 1)
			{
				nk_input_button(self->nk, NK_BUTTON_DOUBLE, event->x, event->y, false);
			}
			nk_input_button(self->nk, NK_BUTTON_LEFT, event->x, event->y, false);
			return STOP;
		}
		else if (event->button == SDL_BUTTON_MIDDLE)
		{
			nk_input_button(self->nk, NK_BUTTON_MIDDLE, event->x, event->y, false);
			return STOP;
		}
		else if (event->button == SDL_BUTTON_RIGHT)
		{
			nk_input_button(self->nk, NK_BUTTON_RIGHT, event->x, event->y, false);
			return STOP;
		}
	}
	return CONTINUE;
}

int32_t c_menu_key_up(c_menu_t *self, SDL_Keycode *key)
{
	if ((char)*key == '1')
	{
		c_charlook_t *charlook = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0);
		c_camera_t *camera = c_camera(charlook);
		if (self->control)
		{
			self->control = false;
			c_mouse_deactivate(c_mouse(self));
			camera->renderer = self->game_renderer;
			return STOP;
		}
		else
		{
			self->control = true;
			if (!self->initiated)
			{
				self->game_renderer = camera->renderer;
				self->menu_renderer = menu_renderer_new(self);

				self->initiated = true;
				loader_push_wait(g_candle->loader, (loader_cb)c_menu_activate_loader,
								 NULL, (c_t*)self);
			}
			c_mouse_activate(c_mouse(self));
			camera->renderer = self->menu_renderer;
		}
	}
	if (!self->control) return CONTINUE;

	if (self->nk)
	{
		if (nk_window_is_any_hovered(self->nk) || nk_item_is_any_active(self->nk))
		{
			if (nk_can_handle_key(self->nk, *key, false))
				return STOP;
		}
	}
	return STOP;
}

int32_t c_menu_key_down(c_menu_t *self, SDL_Keycode *key)
{
	if (!self->control) return CONTINUE;
	if (self->nk)
	{
		if (nk_window_is_any_hovered(self->nk) || nk_item_is_any_active(self->nk))
		{
			nk_can_handle_key(self->nk, *key, true);
			return STOP;
		}
	}
	return STOP;
}

extern float g_motion_power;
extern float g_ssao_power;
extern float g_ssr_power;
extern bool_t g_squints_active;

int32_t c_menu_draw(c_menu_t *self)
{
	if (!self->control)
	{
		return CONTINUE;
	}
	if (self->nk && self->game_renderer && self->game_renderer->output
	    && self->font0 && self->font1)
	{
		int32_t res;
		c_window_t *window = c_window(&SYS);
		struct nk_context *ctx = self->nk;
		struct nk_style *s = &ctx->style;
		texture_t *back = NULL;
		back = renderer_tex(self->game_renderer, ref("refr"));
		struct nk_image background = nk_image_id(back->bufs[back->prev_id].id);
		background.w = window->width;
		background.h = window->height;
		background.region[0] = 0;
		background.region[1] = 0;
		background.region[2] = window->width;
		background.region[3] = window->height;
		s->window.fixed_background = nk_style_item_image(background);
		s->window.background = (struct nk_color){200, 200, 200, 100};

		float w = window->width * 0.4f;
		float h = window->height - 200.0f;
		nk_style_set_font(ctx, &(((struct nk_font*)self->font0)->handle));

		res = nk_can_begin_titled(self->nk, "menu", "menu",
				nk_rect(window->width / 2.0f - w / 2.0f, 100.0f, w, h), NK_WINDOW_BORDER);
		if (res)
		{
			const bool_t was_motion_blur_active = g_motion_power > 0.05f;
			const bool_t was_ssao_active = g_ssao_power > 0.05f;

			nk_layout_row_dynamic(self->nk, 80, 1);
			nk_label(self->nk, "Paused", NK_TEXT_CENTERED);

			nk_style_set_font(ctx, &(((struct nk_font*)self->font1)->handle));
			nk_layout_row_dynamic(self->nk, 30, 2);
			nk_label(self->nk, "pee pee", NK_TEXT_ALIGN_LEFT);
			if (nk_button_label(self->nk, "poo poo"))
			{
			}

			nk_layout_row_dynamic(self->nk, 30, 2);
			nk_label(self->nk, "motion blur", NK_TEXT_ALIGN_LEFT);
			if (nk_slider_float(self->nk, 0.0, &g_motion_power, 1.0, 0.1))
			{
				const bool_t is_motion_blur_active = g_motion_power > 0.05f;
				if (was_motion_blur_active != is_motion_blur_active)
				{
					renderer_toggle_pass(self->game_renderer, ref("motion blur"), is_motion_blur_active);
					renderer_toggle_pass(self->game_renderer, ref("output_motion"), is_motion_blur_active);
				}
			}

			nk_layout_row_dynamic(self->nk, 30, 2);
			nk_label(self->nk, "ambient occlusion", NK_TEXT_ALIGN_LEFT);
			if (nk_slider_float(self->nk, 0.0, &g_ssao_power, 1.0, 0.1))
			{
				const bool_t is_ssao_active = g_ssao_power > 0.05f;
				if (was_ssao_active != is_ssao_active)
				{
					renderer_toggle_pass(self->game_renderer, ref("ssao_pass"), is_ssao_active);
				}
			}

			nk_layout_row_dynamic(self->nk, 30, 2);
			nk_label(self->nk, "exposure squints", NK_TEXT_ALIGN_LEFT);
			if (nk_button_label(self->nk, g_squints_active ? "on" : "off"))
			{
				g_squints_active ^= 1u;
				renderer_toggle_pass(self->game_renderer, ref("luminance_calc"),
				                     g_squints_active);
			}

			nk_layout_row_dynamic(self->nk, 30, 2);
			nk_label(self->nk, "reflections", NK_TEXT_ALIGN_LEFT);
			nk_slider_float(self->nk, 0.0, &g_ssr_power, 1.0, 0.1);

			nk_layout_row_dynamic(self->nk, 40, 1);

			nk_layout_row_dynamic(self->nk, 200, 1);
			const char txt[] =  "imagine wiping back to front and smearing poo all over your balls, lol what a mess!";
			nk_text_wrap(self->nk, txt, strlen(txt));
		}
		nk_end(self->nk);
		nk_can_render(NK_ANTI_ALIASING_ON);
	}
	return CONTINUE;
}

int32_t c_menu_events_begin(c_menu_t *self)
{
	if(self->nk) nk_input_begin(self->nk);
	return CONTINUE;
}

int32_t c_menu_events_end(c_menu_t *self)
{
	if(self->nk) nk_input_end(self->nk);
	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("menu", sizeof(c_menu_t), (init_cb)c_menu_init,
			NULL, 2, ref("node"), ref("mouse"));

	signal_init(ref("component_menu"), sizeof(struct nk_context*));
	signal_init(ref("component_tool"), sizeof(void*));
	signal_init(ref("pick_file_save"), sizeof(void*));
	signal_init(ref("pick_file_load"), sizeof(void*));
	signal_init(ref("transform_start"), sizeof(void*));
	signal_init(ref("transform_stop"), sizeof(void*));

	ct_listener(ct, WORLD, 10, ref("key_up"), c_menu_key_up);

	ct_listener(ct, WORLD, 10, ref("key_down"), c_menu_key_down);

	ct_listener(ct, WORLD, 0, ref("mouse_move"), c_menu_mouse_move);

	ct_listener(ct, WORLD, 0, ref("world_draw"), c_menu_draw);

	ct_listener(ct, WORLD, 0, ref("mouse_press"), c_menu_mouse_press);

	ct_listener(ct, WORLD, 0, ref("mouse_release"), c_menu_mouse_release);

	ct_listener(ct, WORLD, 0, ref("events_begin"), c_menu_events_begin);

	ct_listener(ct, WORLD, 0, ref("events_end"), c_menu_events_end);

	/* ct_listener(ct, WORLD, 0, ref("window_resize"), c_menu_resize); */
}

