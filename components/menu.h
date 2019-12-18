#ifndef MENU_H
#define MENU_H

#include <ecs/ecm.h>
#include <utils/renderer.h>

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
} c_menu_t;

DEF_CASTER("menu", c_menu, c_menu_t)

c_menu_t *c_menu_new(void);

#endif /* !MENU_H */
