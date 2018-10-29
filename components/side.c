#include "side.h"
#include "state.h"
#include "grid.h"
#include "charlook.h"
#include <stdlib.h>
#include <components/node.h>
#include <components/model.h>
#include <components/light.h>
#include <components/name.h>


void c_side_init(c_side_t *self)
{
	self->side = 0;
}

c_side_t *c_side_new(int side, int locked)
{
	c_side_t *self = component_new("side");

	self->locked = locked;
	self->side = side;
	return self;
}

static int c_side_changed(c_side_t *self, int *side)
{
	if(self->side == 2) return CONTINUE;
	int none = *side == -1;
	int visible = !none && ((*side) & 1) == (self->side & 1);

	/* if(c_name(self) && !strcmp(c_name(self)->name, "light")) */
	/* { */
		/* printf("%d\n", self->side); */
	/* } */

	c_model_t *mc = c_model(self);
	if(mc)
	{
		c_model_set_visible(mc, visible);
	}
	c_light_t *lc = c_light(self);
	if(lc)
	{
		c_light_visible(lc, visible);
	}

	return CONTINUE;
}

static int c_side_position_changed(c_side_t *self)
{
	if(self->locked) return CONTINUE;
	c_state_t *state = c_state(&SYS);

	if(state)
	{
		vec3_t pos = vec3_round(c_node_local_to_global(c_node(self), vec3(0.0f)));

		c_grid_t *gc = c_grid(&state->grid);
		if(gc)
		{
			int side = c_grid_get(gc, _vec3(pos));
			if(side != self->side)
			{
				c_charlook_t *fc = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0);

				self->side = side;
				c_side_changed(self, &fc->side);
			}
		}
	}

	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("side", sizeof(c_side_t), c_side_init, NULL, 1, ref("node"));
	ct_listener(ct, WORLD, sig("side_changed"), c_side_changed);
	ct_listener(ct, ENTITY, sig("node_changed"), c_side_position_changed);
}


