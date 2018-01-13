#ifndef GRID_H
#define GRID_H

#include <ecm.h>

typedef struct
{
	c_t super; /* extends c_t */

	int mx, my, mz;
	int *map;

	entity_t blocks, cage, boxes,
			 blocks_inv, cage_inv, boxes_inv;
} c_grid_t;

extern unsigned long ct_grid;
extern unsigned long grid_update;

DEF_CASTER(ct_grid, c_grid, c_grid_t);

c_grid_t *c_grid_new(ecm_t *ecm, int mx, int my, int mz);

int c_grid_get(c_grid_t *self, int x, int y, int z);
void c_grid_set(c_grid_t *self, int x, int y, int z, int val);
void c_grid_print(c_grid_t *self);
void c_grid_register(ecm_t *ecm);
float c_grid_collider(c_grid_t *self, vec3_t pos);

#endif /* !GRID_H */
