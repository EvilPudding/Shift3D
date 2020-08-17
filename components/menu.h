#ifndef MENU_H
#define MENU_H

#include "../candle/ecs/ecm.h"
#include "../candle/utils/renderer.h"

typedef struct c_menu_t
{
	c_t super;
	/* currently, menu has no options */
	bool_t initiated;
	void *context;
	void *nk;
	renderer_t *game_renderer;
	renderer_t *menu_renderer;
	bool_t control;
	void *font0, *font1;

	float motion_power;
	float ssao_power;
	float ssr_power;
	bool_t squints_active;
} c_menu_t;

DEF_CASTER(ct_menu, c_menu, c_menu_t)

c_menu_t *c_menu_new(void);

#endif /* !MENU_H */
