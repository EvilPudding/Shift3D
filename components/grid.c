#include "../candle/ecs/ecm.h"
#include "../candle/candle.h"
#include "../candle/components/name.h"
#include "../candle/components/model.h"
#include "../candle/components/decal.h"
#include "../candle/components/spatial.h"
#include "../candle/components/node.h"
#include "../candle/components/camera.h"
#include "../candle/systems/editmode.h"

#include "rigid_body.h"
#include "grid.h"
#include "level.h"
#include "side.h"
#include "character.h"
#include "movable.h"
#include <stdlib.h>
#include <stdarg.h>

void mesh_add_spike(mesh_t *self, float s, vec3_t v, vec3_t dir, int inverted_normals);
/* void mesh_add_spike(mesh_t *self, float s, float x, float y, float z, */
		/* int px, int py, int pz, int inverted_normals); */
void mesh_add_plane(mesh_t *self, float s, vec3_t v, vec3_t dir, int inverted_normals);
/* void mesh_add_plane(mesh_t *self, float s, float x, float y, float z, */
		/* int px, int py, int pz, int inverted_normals); */

typedef void(*create_cb)(mesh_t *self, float s, vec3_t v, vec3_t dir,
		int inverted_normals);
/* typedef void(*create_cb)(mesh_t *self, float s, float x, float y, float z, */
		/* int px, int py, int pz, int inverted_normals); */

static int c_grid_update(c_grid_t *self);
mesh_t *mesh_from_grid(mesh_t *self, c_grid_t *grid, int side, int flags, ...);
int plane_to_side(mesh_t *mesh, int val0, int flag, c_grid_t *grid,
		vec3_t v, vec3_t dir, create_cb create);

/* int cmd_model_before_draw(c_model_t *self); */

float c_rigid_body_grid_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_grid_t *g = c_grid(self);
	if(g->active != 1) return -1;

	c_character_t *fc = (c_character_t*)ct_get_nth(ecm_get(ct_character), 0);
	int side = c_side(fc)->side & 1;

	pos = vec3_round(pos);
	int val = c_grid_get(g, _vec3(pos));

	int ret = ((val&1) != side || val == -1) ? 0 : -1;

	return ret;
}

static int c_grid_created(c_grid_t *self)
{
	c_side_t *ss = c_side(self);

	self->blocks = entity_new({
		c_name_new("blocks");
		c_side_new(ss->level, 0, 1);
		c_model_new(mesh_new(), sauces("white.mat"), 1, 1);
	});
	strcpy(c_model(&self->blocks)->mesh->name, "blocks");

	self->cage = entity_new({
		c_name_new("cage");
		c_side_new(ss->level, 0, 1);
		c_model_new(mesh_new(), sauces("paint05.mat"), 1, 1);
	});
	strcpy(c_model(&self->cage)->mesh->name, "cage");

	mat_t *stone3 = sauces("stone3.mat");
	mat_t *stone4 = sauces("stone4.mat");
	/* mat1f(stone4, ref("albedo.blend"), 0.5); */
	/* mat1f(stone4, ref("normal.blend"), 0.3); */


	self->boxes = entity_new({
		c_name_new("movable");
		c_side_new(ss->level, 0, 1);
		c_model_new(mesh_new(), stone4, 1, 1);
	});
	strcpy(c_model(&self->boxes)->mesh->name, "boxes");

	self->blocks_inv = entity_new({
		c_name_new("bloc_i");
		c_side_new(ss->level, 1, 1);
		c_model_new(mesh_new(), sauces("black.mat"), 1, 1);
	});
	strcpy(c_model(&self->blocks_inv)->mesh->name, "boxes_inv");

	self->cage_inv = entity_new({
		c_name_new("cage_i");
		c_side_new(ss->level, 1, 1);
		c_model_new(mesh_new(), sauces("paint01.mat"), 1, 1);
	});
	strcpy(c_model(&self->cage_inv)->mesh->name, "cage_inv");

	self->boxes_inv = entity_new({
		c_name_new("movab_i");
		c_side_new(ss->level, 1, 1);
		c_model_new(mesh_new(), stone3, 1, 1);
	});
	strcpy(c_model(&self->boxes_inv)->mesh->name, "boxes_inv");

	/* c_model(&self->blocks_inv)->before_draw = */
	/* 	c_model(&self->cage_inv)->before_draw = */
	/* 	c_model(&self->boxes_inv)->before_draw = */
	/* 	(before_draw_cb)cmd_model_before_draw; */

	/* c_model(&self->blocks)->before_draw = */
	/* 	c_model(&self->cage)->before_draw = */
	/* 	c_model(&self->boxes)->before_draw = */
	/* 	(before_draw_cb)cmd_model_before_draw; */

	c_grid_update(self);

	c_node_add(c_node(self), 6, self->boxes_inv, self->cage_inv,
			self->blocks_inv, self->boxes, self->cage, self->blocks);

	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_grid_collider));

	self->modified = 1;

	return CONTINUE;
}

static int32_t grid_edit_init(c_grid_t *self)
{
	if (!entity_exists(self->edit_target))
	{
		printf("edit init\n");
		mat_t *edit_mat = mat_new("edit_mat", "transparent");
		mat_t *remove_mat = mat_new("remove_mat", "decal");
		/* edit_mat->metalness.texture = sauces("rough.png"); */
		/* edit_mat->metalness.color = vec4(1, 1, 1, 1); */
		/* edit_mat->metalness.blend = 0; */
		/* edit_mat->roughness.texture = sauces("rough.png"); */
		/* edit_mat->roughness.blend = 0.8; */
		/* edit_mat->roughness.scale = 0.2; */
		/* edit_mat->normal.scale = 2.0; */
		/* edit_mat->normal.texture = sauces("stone3_normal.tga"); */
		/* edit_mat->normal.blend = 1; */
		mat4f(edit_mat, ref("absorb.color"), vec4(0.3f, 0.3f, 0.0f, 1.0));
		mat4f(remove_mat, ref("albedo.color"), vec4(1.0f, 0.3f, 0.0f, 1.0));

		mesh_t *cube = mesh_new();
		mesh_cube(cube, 0.5f, 1.0f);
		self->edit_target = entity_new(c_model_new(cube, edit_mat, 0, 1));
		self->remove_target = entity_new(c_decal_new(remove_mat, 1, 0));
		c_spatial_scale(c_spatial(&self->remove_target), vec3(0.8, 0.8, 0.8));

		c_model_t *mc = c_model(&self->edit_target);
		c_model_set_groups(mc, mc->visible_group, mc->shadow_group,
				mc->transparent_group, 0);

		c_node(&self->remove_target)->ghost = 1;
		c_node(&self->edit_target)->ghost = 1;
	}
	c_model_set_visible(c_model(&self->edit_target), 1);
	return CONTINUE;
}

static int32_t grid_edit_move(c_grid_t *self, vec3_t p, c_editmode_t *ec)
{
	c_node_t *cam = c_node(&ec->camera);
	vec3_t cam_pos = c_node_pos_to_global(cam, vec3(0.0f, 0.0f, 0.0f));

	/* vec3_t alt; */
	/* p = vec3_add(p, vec3_scale(dir, 0.05f / vec3_len(dir))); */

	/* const vec3_t alt[6] = { */
	/* 	vec3( 1.0f, 0.0f, 0.0f), */
	/* 	vec3(-1.0f, 0.0f, 0.0f), */
	/* 	vec3( 0.0f, 1.0f, 0.0f), */
	/* 	vec3( 0.0f,-1.0f, 0.0f), */
	/* 	vec3( 0.0f, 0.0f, 1.0f), */
	/* 	vec3( 0.0f, 0.0f,-1.0f) */
	/* }; */
	/* uint32_t closest_dir = 0; */
	/* for (uint32_t i = 1; i < 6; ++i) */
	/* { */
	/* 	if (vec3_dot(dir, alt[closest_dir]) > vec3_dot(dir, alt[i])) */
	/* 	{ */
	/* 		closest_dir = i; */
	/* 	} */
	/* } */

	vec3_t pround = vec3_round(p);
	/* const bool_t side = c_grid_get(self, _vec3(cam_pos)) & 1; */
	vec3_t dif = vec3_sub(p, pround);
	vec3_t palt = pround;
	if (fabs(dif.x) > fabs(dif.y))
	{
		if (fabs(dif.x) > fabs(dif.z))
		{
			palt.x += dif.x > 0.0f ? 1.0f : -1.0f;
		}
		else
		{
			palt.z += dif.z > 0.0f ? 1.0f : -1.0f;
		}
	}
	else if (fabs(dif.y) > fabs(dif.z) * 1.0f)
	{
		palt.y += dif.y > 0.0f ? 1.0f : -1.0f;
	}
	else
	{
		palt.z += dif.z > 0.0f ? 1.0f : -1.0f;
	}
	vec3_t rem, add;
	if (vec3_dist(palt, cam_pos) > vec3_dist(cam_pos, pround))
	{
		rem = palt;
		add = pround;
	}
	else
	{
		rem = pround;
		add = palt;
	}

	rem = vec3_add(rem, vec3_scale(vec3_sub(add, rem), 0.3f));
	c_spatial_set_pos(c_spatial(&self->remove_target), rem);
	c_spatial_set_pos(c_spatial(&self->edit_target), add);
	return CONTINUE;
}

static int32_t grid_edit_release(c_grid_t *self, vec3_t p, int32_t button,
                                 c_editmode_t *ec)
{
	if(button == CANDLE_MOUSE_BUTTON_LEFT)
	{
		const vec3_t add = c_spatial(&self->edit_target)->pos;
		c_grid_set(self, _vec3(add), 1);
	}
	else
	{
		const vec3_t rm = c_spatial(&self->remove_target)->pos;
		c_grid_set(self, _vec3(rm), 0);
	}

	c_grid_update(self);

	return STOP;
}

static int c_grid_editmode_toggle(c_grid_t *self)
{
	c_editmode_t *edit = c_editmode(&SYS);
	if(!edit) return CONTINUE;

	if(edit->control)
	{
		c_editmode_add_tool(c_editmode(&SYS), 'e', "grid_edit",
		                    (mouse_tool_init_cb)grid_edit_init,
		                    (mouse_tool_move_cb)grid_edit_move, NULL, NULL,
		                    (mouse_tool_release_cb)grid_edit_release, NULL, NULL,
		                    self, ref("grid"));

	}

	return CONTINUE;
}

void ct_grid(ct_t *self)
{
	ct_init(self, "grid", sizeof(c_grid_t));
	ct_add_dependency(self, ct_node);

	ct_add_listener(self, WORLD, 0, ref("grid_update"), c_grid_update);
	ct_add_listener(self, ENTITY, 0, ref("entity_created"), c_grid_created);
	ct_add_listener(self, WORLD, 0, ref("editmode_toggle"), c_grid_editmode_toggle);

}

c_grid_t *c_grid_new(int mx, int my, int mz)
{
	c_grid_t *self = component_new(ct_grid);

	self->mx = mx;
	self->my = my;
	self->mz = mz;
	self->map = calloc(mx * my * mz, sizeof(*self->map));
	return self;
}

void mesh_add_spike(mesh_t *mesh, float s, vec3_t v, vec3_t dir, int inverted_normals)
{
	/* return; */
	mat4_t save = mesh_save(mesh);

	mesh_translate(mesh, v);
    if(dir.x > 0)
	{
		mesh_rotate(mesh, 90, 0, 0, 1);
	}
	else if(dir.x < 0)
	{
		mesh_rotate(mesh, -90, 0, 0, 1);
	}
	else if(dir.y > 0)
	{
		mesh_rotate(mesh, 180, 1, 0, 0);
	}
	else if(dir.y < 0)
	{
		/* mesh_rotate(mesh, 180, 0, 0, 0); */
	}
	else if(dir.z > 0)
	{
		mesh_rotate(mesh, -90, 1, 0, 0);
	}
	else if(dir.z < 0)
	{
		mesh_rotate(mesh, 90, 1, 0, 0);
	}
	mesh_translate(mesh, vec3(0, -0.5, 0));

	int v1, v2, v3;
	/* mesh_add_plane(mesh, s, 0, 0, 0, 0, -1, 1); */
	/* mesh_restore(mesh); */
	/* return; */

	float i, j;
	int U, V;

	/* float wid = 0.25; */
	const float wid = 0.5;
	const float hei = 0.6;
    for(i=-wid, U=-2; i<wid; i+=wid,U++)
		for(j=-wid, V=-2; j<wid; j+=wid,V++)
	{

		float mainx = i + wid/2;
		float mainy = (U + V) & 1 ? hei : -hei;
		float mainz = j + wid/2;

		vec3_t zero = vec3(0.0f, 0.0f, 0.0f);

		vec3_t P1 = vec3(mainx, mainy, mainz);
		vec3_t P2 = vec3(i, 0, j+wid);
		vec3_t P3 = vec3(i+wid, 0, j+wid);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh, 
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z));

		P1 = vec3(mainx, mainy, mainz);
		P2 = vec3(i+wid, 0, j+wid);
		P3 = vec3(i+wid, 0, j);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh,
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z));

		P1 = vec3(mainx, mainy, mainz);
		P2 = vec3(i+wid, 0, j);
		P3 = vec3(i, 0, j);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh,
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z));

		P1 = vec3(mainx, mainy, mainz);
		P2 = vec3(i, 0, j);
		P3 = vec3(i, 0, j+wid);
		v1 = mesh_add_vert(mesh, P1);
		v2 = mesh_add_vert(mesh, P2);
		v3 = mesh_add_vert(mesh, P3);
		mesh_add_triangle(mesh,
				v1, zero, vec2(P1.x, P1.z),
				v2, zero, vec2(P2.x, P2.z),
				v3, zero, vec2(P3.x, P3.z));

	}
	mesh_restore(mesh, save);
	mesh_modified(mesh);
}


void mesh_add_box(mesh_t *self, float s, vec3_t v, vec3_t dir, int invert_normals)
{
	mesh_lock(self);
	mat4_t save = mesh_save(self);

	mesh_translate(self, v);
    if(dir.x > 0)
	{
		mesh_rotate(self, 90, 0, 0, 1);
		mesh_rotate(self, 90, 0, 1, 0);
	}
	else if(dir.x < 0)
	{
		mesh_rotate(self, -90, 0, 0, 1);
		mesh_rotate(self, -90, 0, 1, 0);
	}
	else if(dir.y > 0)
	{
		mesh_rotate(self, 180, 1, 0, 0);
	}
	else if(dir.y < 0)
	{
		/* mesh_rotate(self, 180, 1, 0, 0); */
	}
	else if(dir.z > 0)
	{
		mesh_rotate(self, -90, 1, 0, 0);
	}
	else if(dir.z < 0)
	{
		mesh_rotate(self, 90, 1, 0, 0);
		mesh_rotate(self, 180, 0, 1, 0);
	}

	vec2_t v1t, v2t, v3t, v4t;
	v1t = vec2(+s, -s);
	v2t = vec2(-s, -s);
	v3t = vec2(-s, +s);
	v4t = vec2(+s, +s);
	/* mesh_circle(self, 0.5f, 20, n); */

	vec3_t n = vec3(0.0f, 1.0f, 0.0f);
	self->smooth_angle = 0.0f;

	mesh_unselect(self, SEL_EDITING, MESH_ANY, -1);
	int e0 = mesh_add_edge(self, mesh_add_vert(self, vec3(+s, -s, +s)), -1, -1, n, v4t);
	mesh_select(self, SEL_EDITING, MESH_EDGE, e0);
	int e1 = mesh_add_edge(self, mesh_add_vert(self, vec3(-s, -s, +s)), -1, e0, n, v3t);
	mesh_select(self, SEL_EDITING, MESH_EDGE, e1);
	int e2 = mesh_add_edge(self, mesh_add_vert(self, vec3(-s, -s, -s)), -1, e1, n, v2t);
	mesh_select(self, SEL_EDITING, MESH_EDGE, e2);
	int e3 = mesh_add_edge(self, mesh_add_vert(self, vec3(+s, -s, -s)), e0, e2, n, v1t);
	mesh_select(self, SEL_EDITING, MESH_EDGE, e3);
	/* mesh_circle(self, 1.0f, 4, n); */

	mesh_restore(self, save);
	/* self->has_texcoords = 0; */

	mesh_extrude_edges(self, 1, vec3(0.0f, 0.0f, 0.0f), 0.8f, NULL, NULL, NULL);
	mesh_extrude_edges(self, 1, vec3_scale(dir, -0.08f * s), 1.0f, NULL, NULL, NULL);
	mesh_extrude_edges(self, 1, vec3(0.0f, 0.0f, 0.0f), 0.9f, NULL, NULL, NULL);
	mesh_extrude_edges(self, 1, vec3_scale(dir, -0.08f * s), 1.0f, NULL, NULL, NULL);
	mesh_extrude_edges(self, 1, vec3(0.0f, 0.0f, 0.0f), 0.9f, NULL, NULL, NULL);
	mesh_extrude_edges(self, 1, vec3_scale(dir, 0.16f * s), 1.0f, NULL, NULL, NULL);
	mesh_extrude_edges(self, 1, vec3(0.0f, 0.0f, 0.0f), 0.001f, NULL, NULL, NULL);

	mesh_unselect(self, SEL_EDITING, MESH_ANY, -1);

	mesh_modified(self);
	mesh_unlock(self);
}

void mesh_add_plane(mesh_t *self, float s, vec3_t v, vec3_t dir, int invert_normals)
{
	int v1, v2, v3, v4;

	mat4_t save = mesh_save(self);

	mesh_translate(self, v);
    if(dir.x > 0)
	{
		mesh_rotate(self, 90, 0, 0, 1);
		mesh_rotate(self, 90, 0, 1, 0);
	}
	else if(dir.x < 0)
	{
		mesh_rotate(self, -90, 0, 0, 1);
		mesh_rotate(self, -90, 0, 1, 0);
	}
	else if(dir.y > 0)
	{
		mesh_rotate(self, 180, 1, 0, 0);
	}
	else if(dir.y < 0)
	{
		/* mesh_rotate(self, 180, 1, 0, 0); */
	}
	else if(dir.z > 0)
	{
		mesh_rotate(self, -90, 1, 0, 0);
	}
	else if(dir.z < 0)
	{
		mesh_rotate(self, 90, 1, 0, 0);
		mesh_rotate(self, 180, 0, 1, 0);
	}

	vec2_t v1t, v2t, v3t, v4t;
		v1t = vec2(+s, -s);
		v2t = vec2(-s, -s);
		v3t = vec2(-s, +s);
		v4t = vec2(+s, +s);
		v1 = mesh_add_vert(self, vec3(+s, -s, -s));
		v2 = mesh_add_vert(self, vec3(-s, -s, -s));
		v3 = mesh_add_vert(self, vec3(-s, -s, +s));
		v4 = mesh_add_vert(self, vec3(+s, -s, +s));
	mesh_add_quad(self,
			v1, vec3(0.0f, 0.0f, 0.0f), v1t,
			v2, vec3(0.0f, 0.0f, 0.0f), v2t,
			v3, vec3(0.0f, 0.0f, 0.0f), v3t,
			v4, vec3(0.0f, 0.0f, 0.0f), v4t);
	mesh_restore(self, save);
	mesh_modified(self);
}

void c_grid_set(c_grid_t *self, int x, int y, int z, int val)
{
	if(x < 0 || x >= self->mx ||
			y < 0 || y >= self->my ||
			z < 0 || z >= self->mz) return;
	self->modified = 1;

	self->map[z + (y * self->mz + x) * self->mx] = val;

}

int c_grid_get(c_grid_t *self, int x, int y, int z)
{
	if(x < 0 || x >= self->mx ||
			y < 0 || y >= self->my ||
			z < 0 || z >= self->mz) return -1;

	return self->map[z + (y * self->mz + x) * self->mx];
}

int32_t grid_edit_end(c_grid_t *self)
{
	c_model_set_visible(c_model(&self->edit_target), 0);
	return CONTINUE;
}


static int c_grid_update(c_grid_t *self)
{
	/* if(!self->modified) return CONTINUE; */
	self->modified = 0;

	mesh_t *new_terrainA = c_model(&self->blocks)->mesh;
	mesh_t *new_cageA = c_model(&self->cage)->mesh;
	mesh_t *new_boxesA = c_model(&self->boxes)->mesh;
	mesh_t *new_terrainB = c_model(&self->blocks_inv)->mesh;
	mesh_t *new_cageB = c_model(&self->cage_inv)->mesh;
	mesh_t *new_boxesB = c_model(&self->boxes_inv)->mesh;

	mesh_from_grid(new_terrainA, self, 0, 2, 0x4, mesh_add_spike, 0x0, mesh_add_plane);

	mesh_from_grid(new_terrainB, self, 1, 2, 0x4, mesh_add_spike, 0x0, mesh_add_plane);

	mesh_from_grid(new_cageA, self, 0, 1, -1, mesh_add_plane);

	mesh_from_grid(new_cageB, self, 1, 1, -1, mesh_add_plane);

	mesh_from_grid(new_boxesA, self, 0, 1, 0x2, mesh_add_box);

	mesh_from_grid(new_boxesB, self, 1, 1, 0x2, mesh_add_box);

	/* new_terrainB->cull = 2; */
	/* new_boxesB->cull = 1; */

	entity_signal(c_entity(self), sig("spatial_changed"), &self->blocks, NULL);

	/* c_grid_print(self); */

	return CONTINUE;
}

void c_grid_print(c_grid_t *self)
{
	int x, y, z;
    for(x = 0; x < self->mx; x++)
	{
		for(y = 0; y < self->my; y++)
		{
			for(z = 0; z < self->mz; z++)
			{
				int val = c_grid_get(self, x, y, z);
				printf("%d ", val);
			}
			printf("\n");
		}
		printf("\n");
	}
}

mesh_t *mesh_from_grid(mesh_t *self, c_grid_t *grid, int side, int flags, ...)
{
	/* self->wireframe = 1; */
	int x, y, z;
	mesh_lock(self);
	mesh_clear(self);

	va_list list;

    for(x = 0; x < grid->mx; x++)
		for(y = 0; y < grid->my; y++)
			for(z = 0; z < grid->mz; z++)
	{
		int val = c_grid_get(grid, x, y, z);

		if((val & 1) != side)
		{
			if(val&2)
			{
				val = side;
			}
			else
			{
				continue;
			}
		}

		int j, r1 = 0, r2 = 0, r3 = 0, r4 = 0, r5 = 0, r6 = 0;
		va_start(list, flags);
		for(j = 0; j < flags; j++)
		{
			int flag = va_arg(list, int);
			create_cb create = va_arg(list, create_cb);


			if((val & ~0x1) == flag || flag == -1 || (val & ~0x1) == 0x4)
			{
				if(!r1) r1 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3(-1,  0,  0), create);
				if(!r2) r2 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0, -1,  0), create);
				if(!r3) r3 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0,  0, -1), create);

				if(!r4) r4 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 1,  0,  0), create);
				if(!r5) r5 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0,  1,  0), create);
				if(!r6) r6 = plane_to_side(self, val, flag, grid, vec3(x, y, z), vec3( 0,  0,  1), create);
			}
		}
		va_end(list);
	}
	mesh_translate_uv(self, vec2(0.5, 0.5));
	/* mesh_scale_uv(self, 0.8); */
	/* mesh_scale_uv(self, 0.5); */

	mesh_unlock(self);
	return self;
}

int plane_to_side(mesh_t *mesh, int val0, int flag, c_grid_t *grid,
		vec3_t v, vec3_t dir, create_cb create)
{
	int val = c_grid_get(grid, v.x + dir.x, v.y + dir.y, v.z + dir.z);
	if((val & 0x1) == (val0 & 0x1) && flag != -1) return 0;

	if(val == -1)
	{
		if(flag == -1)
		{
			create(mesh, 0.5, v, dir, 0);
		}
		return 1;
	}
	if(val & 2)
	{
		return 1;
	}
	if(flag == 2)
	{
		if(val0 & 2)
		{
			create(mesh, 0.5, v, dir, 0);
			return 1;
		}
		return 0;
	}
	if((val & ~0x1) == flag && flag)
	{
		create(mesh, 0.5, v, dir, 0);
		return 1;
	}
	if(flag == 0 && val != -1)
	{
		create(mesh, 0.5, v, dir, 0);
		return 1;
	}
	return 0;
}

int grid_transverse(c_grid_t *self, vec3_t curpos, vec3_t raydir)
{

	int retval = -1;
	// setup 3DDDA (double check reusability of primary ray data)
	vec3_t cb, tmax, tdelta = vec3(0.0f, 0.0f, 0.0f), cell;
	cell = curpos;
	int stepX, outX, X = (int)cell.x;
	int stepY, outY, Y = (int)cell.y;
	int stepZ, outZ, Z = (int)cell.z;
	if ((X < 0) || (X >= self->mx) || (Y < 0) || (Y >= self->my) || (Z < 0) || (Z >= self->mz)) return 0;
	if (raydir.x > 0)
	{
		stepX = 1, outX = self->mx;
		cb.x = (X + 1) * self->mx;
	}
	else 
	{
		stepX = -1, outX = -1;
		cb.x = X;
	}
	if (raydir.y > 0.0f)
	{
		stepY = 1, outY = self->my;
		cb.y = (Y + 1); 
	}
	else 
	{
		stepY = -1, outY = -1;
		cb.y = Y;
	}
	if (raydir.z > 0.0f)
	{
		stepZ = 1, outZ = self->mz;
		cb.z = (Z + 1);
	}
	else 
	{
		stepZ = -1, outZ = -1;
		cb.z = Z;
	}
	float rxr, ryr, rzr;
	if (raydir.x != 0)
	{
		rxr = 1.0f / raydir.x;
		tmax.x = (cb.x - curpos.x) * rxr; 
		tdelta.x = stepX * rxr;
	}
	else tmax.x = 1000000;

	if (raydir.y != 0)
	{
		ryr = 1.0f / raydir.y;
		tmax.y = (cb.y - curpos.y) * ryr; 
		tdelta.y = stepY * ryr;
	}
	else tmax.y = 1000000;

	if (raydir.z != 0)
	{
		rzr = 1.0f / raydir.z;
		tmax.z = (cb.z - curpos.z) * rzr; 
		tdelta.z = stepZ * rzr;
	}
	else tmax.z = 1000000;

	// start stepping
	int val = 0;
	// trace primary ray
	while (1)
	{
		val = c_grid_get(self, X, Y, Z);
		if(val)
		{
			return val;
		}
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				X = X + stepX;
				if (X == outX) return -1;
				tmax.x += tdelta.x;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return -1;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				Y = Y + stepY;
				if (Y == outY) return -1;
				tmax.y += tdelta.y;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return -1;
				tmax.z += tdelta.z;
			}
		}
	}
	return retval;
}
