#include "templates.h"
#include "components/side.h"
#include "components/grid.h"
#include "components/key.h"
#include "components/bridge.h"
#include "components/door.h"
#include "components/level.h"
#include "components/character.h"
#include "components/charlook.h"
#include "components/side_follow.h"
#include <stdio.h>
#include <stdlib.h>

int template_model_before_draw(c_model_t *self)
{
	c_side_t *sc = c_side(self);
	if(!sc) return 1;
	int side = sc->side;
	if(side != c_side(&candle->systems)->side && side != 2)
	{
		/* printf("'%s' on wrong side\n", c_name(self->super.entity)->name); */
		return 0;
	}
	return 1;
}

/* static int template_light_before_draw(c_light_t *self) */
/* { */
/* 	int side = c_side(self->super.entity)->side; */
/* 	return side == 2 || side == c_side(&candle->systems)->side; */
/* } */

static entity_t template_shift_grid(entity_t root, FILE *fd)
{
	int ret;
	int mx, my, mz;
	int i;
	char c;
	int l;
	entity_t entity;

	ret = fscanf(fd, "%d %d %d ", &mx, &my, &mz);
	if(!ret) exit(1);

	entity = entity_new(c_name_new("grid"), c_node_new(), c_grid_new(mx, my, mz));

	c_grid_t *grid = c_grid(&entity);

	l = mx * my * mz;

	for(i = 0; (c = getc(fd)) != EOF;)
	{
		grid->map[i++] = (c >> 0) & (~0x88) & (0x0F);

		if(i == l) break;

		grid->map[i++] = (c >> 4) & (~0x88) & (0x0F);

		if(i == l) break;
	}

	getc(fd);

	entity_signal(entity, grid_update, NULL);
	c_node_add(c_node(&root), 1, entity);

	return root;
}

static entity_t template_key(entity_t root, FILE *fd, candle_t *candle)
{
	int ret;
	float x, y, z;
	int rotX, rotY, rotZ;
	int id, side;

	ret = fscanf(fd, "%f %f %f %d %d %d %d ", &x, &y, &z, &rotX, &rotY, &rotZ, &id);
	if(!ret) exit(1);

	entity_t grid = c_node_get_by_name(c_node(&root), "grid");
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	entity_t key = entity_new(c_name_new("key"),
			c_node_new(),
			c_model_paint(c_model_new(sauces_mesh("key.obj"), 1), 0,
				sauces_mat("key")),
			c_side_new(side),
			c_key_new(rotX, rotY, rotZ, id));
	c_model(&key)->before_draw = (before_draw_cb)template_model_before_draw;

	c_spacial_set_pos(c_spacial(&key), vec3(x, y, z));

	c_node_add(c_node(&root), 1, key);
	return root;
}

static entity_t template_spawn(entity_t root, FILE *fd, candle_t *candle)
{
	float x, y, z;
	int dir, side;

	int ret = fscanf(fd, "%f %f %f %d ", &x, &y, &z, &dir);
	if(!ret) exit(1);

	entity_t grid = c_node_get_by_name(c_node(&root), "grid");
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	c_side(&candle->systems)->side = side;

	entity_t spawn = entity_new(c_node_new(),
			c_side_new(side),
			c_spacial_new(),
			c_name_new("spawn"));

	c_character_t *fc = (c_character_t*)ct_get_at(ecm_get(ct_character), 0, 0);

	/* c_camera_t *cc = c_camera(ecm_get_camera(root.ecm)); */

	if(fc)
	{
		c_spacial_set_pos(c_spacial(fc), vec3(x, y - 0.5, z));
		/* c_spacial_set_pos(c_spacial(cc->super.entity), 0, 0.7, 0); */
	}
	c_spacial_set_pos(c_spacial(&spawn), vec3(x, y, z));

	c_node_add(c_node(&root), 1, spawn);
	return root;
}

static entity_t template_bridge(entity_t root, FILE *fd, candle_t *candle)
{
#define order(N1, N2, n1, n2) ((n1 < n2) ? \
		(N1 = n1, N2 = n2) : \
		(N1 = n2, N2 = n1))

	int x1, y1, z1, x2, y2, z2, cx, cy, cz, key;

	int ret = fscanf(fd, " %d %d %d %d %d %d %d %d %d %d", &x1, &y1, &z1, &x2,
			&y2, &z2, &cx, &cy, &cz, &key);
	if(!ret) exit(1);

	entity_t bridge = entity_new(c_side_new(2),
			c_bridge_new(),
			c_node_new());
	c_bridge_t *p = c_bridge(&bridge);

	order(p->ix1, p->ix2, x1, x2);
	order(p->iy1, p->iy2, y1, y2);
	order(p->iz1, p->iz2, z1, z2);

	p->ix1 -= cx; p->ix2 -= cx;
	p->iy1 -= cy; p->iy2 -= cy;
	p->iz1 -= cz; p->iz2 -= cz;

	p->min.x = p->ix1; p->min.y = p->iy1; p->min.z = p->iz1;
	p->max.x = p->ix2; p->max.y = p->iy2; p->max.z = p->iz2;

	p->min = vec3_sub(p->min, vec3(0.5));
	p->max = vec3_add(p->max, vec3(0.5));

	p->cx=cx;
	p->cy=cy;
	p->cz=cz;

	p->key=key;

	/* mesh_t *mesh = mesh_cuboid(0.5, */
			/* p->x1 - 0.01, p->y1 - 0.01, p->z1 - 0.01, */
			/* p->x2 + 0.01, p->y2 + 0.01, p->z2 + 0.01); */
	c_bridge_ready(p);

	c_spacial_set_pos(c_spacial(&bridge), vec3(p->cx, p->cy, p->cz));

	c_model(&bridge)->before_draw = (before_draw_cb)template_model_before_draw;

	c_node_add(c_node(&root), 1, bridge);
	return root;
}

static entity_t template_door(entity_t root, FILE *fd, candle_t *candle)
{
	int ret;
	float x, y, z;
	char next[256];
	int dir;
	int side;

	ret = fscanf(fd, "%f %f %f %d %s ", &x, &y, &z, &dir, next);
	if(!ret) exit(1);

	entity_t grid = c_node_get_by_name(c_node(&root), "grid");
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	entity_t door = entity_new(c_name_new("door"),
			c_model_paint(c_model_new(sauces_mesh( "door.obj"), 1),
				0, sauces_mat( "door")),
			c_side_new(side),
			c_node_new(),
			c_door_new(next));
	c_model(&door)->before_draw = (before_draw_cb)template_model_before_draw;

	c_spacial_set_pos(c_spacial(&door), vec3(x, y - 0.5 * (side ? -1:1), z));
	c_spacial_set_rot(c_spacial(&door), 0, 1, 0, dir * M_PI / 2);
	/* c_spacial_set_rot(c_spacial(door), 0, 1, 0, side ? -M_PI / 2.0 : 0); */

	c_node_add(c_node(&root), 1, door);
	return root;
}

static entity_t template_light(entity_t root, FILE *fd)
{
	int ret;
	float x, y, z, intensity;
	int side;
	vec4_t color;

	ret = fscanf(fd, "%f %f %f %f %f %f %f ", &x, &y, &z, &intensity,
			&color.r, &color.g, &color.b);
	if(!ret) exit(1);

	entity_t grid = c_node_get_by_name(c_node(&root), "grid");
	side = c_grid_get(c_grid(&grid), x, y, z) & 1;

	entity_t light = entity_new(c_name_new("light"),
			c_side_new(side),
			c_node_new(),
			c_side_follow_new(),
			c_light_new(intensity, color, 512));

	/* c_light(light)->before_draw = (before_draw_cb)template_light_before_draw; */

	c_spacial_set_pos(c_spacial(&light), vec3(x, y, z));

	c_node_add(c_node(&root), 1, light);
	return root;
}

void register_custom_templates(candle_t *candle)
{
	candle_register_template(candle, "light", (template_cb)template_light);
	candle_register_template(candle, "door", (template_cb)template_door);
	candle_register_template(candle, "bridge", (template_cb)template_bridge);
	candle_register_template(candle, "spawn", (template_cb)template_spawn);
	candle_register_template(candle, "key", (template_cb)template_key);
	candle_register_template(candle, "shift_grid", (template_cb)template_shift_grid);
}
