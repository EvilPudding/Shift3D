#include "cmds.h"
#include "components/bridge.h"
#include "components/character.h"
#include "components/charlook.h"
#include "components/door.h"
#include "components/grid.h"
#include "components/key.h"
#include "components/level.h"
#include "components/menu.h"
#include "components/mirror.h"
#include "components/movable.h"
#include "components/rigid_body.h"
#include "components/side.h"
#include "components/side_follow.h"
#include "components/force.h"

#include "candle/components/model.h"
#include "candle/components/node.h"
#include "candle/components/name.h"
#include "candle/components/light.h"
#include "candle/components/camera.h"
#include "candle/utils/renderer.h"
#include "openal.candle/openal.h"
#include "openal.candle/speaker.h"
#include "openxr.candle/openxr.h"

#include <stdio.h>
#include <stdlib.h>

/* static int cmd_light_before_draw(c_light_t *self) */
/* { */
/* 	int side = c_side(self->super.entity)->side; */
/* 	return side == 2 || side == c_side(&candle->systems)->side; */
/* } */

void mesh_add_box(mesh_t *self, float s, vec3_t v, vec3_t dir, int invert_normals);
static entity_t cmd_shift_grid(entity_t root, int argc, const char **argv)
{
	int mx, my, mz;
	int i;
	/* char c; */
	int l;

	sscanf(argv[1], "%d", &mx);
	sscanf(argv[2], "%d", &my);
	sscanf(argv[3], "%d", &mz);

	c_level_t *level = c_level(&root);
	level->grid = entity_new({
		c_name_new("grid");
		c_grid_new(mx, my, mz);
		c_side_new(root, -1, 1);
	});
	c_grid_t *grid = c_grid(&level->grid);

	l = mx * my * mz;

	for(i = 0; i < l; i++)
	{
		grid->map[i] = argv[4][i] - '0';
	}

	/* mesh_t *mesh = mesh_new(); */
	/* mesh_cube(mesh, 1, 1); */
	mesh_t *mesh = mesh_new();
	/* mesh_cuboid(mesh, 0.5, */
	/* 		vec3(-0.5, -0.5, -0.5), */
	/* 		vec3(0.5, 0.5, 0.5)); */
	mesh_lock(mesh);
	mesh_add_box(mesh, -0.5f, vec3(0.0f, 0.0f, 0.0f), vec3(-1.0f, 0.0f, 0.0f), false);
	mesh_add_box(mesh, -0.5f, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, -1.0f, 0.0f), false);
	mesh_add_box(mesh, -0.5f, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, -1.0f), false);
	mesh_add_box(mesh, -0.5f, vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), false);
	mesh_add_box(mesh, -0.5f, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f), false);
	mesh_add_box(mesh, -0.5f, vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f), false);
	mesh_unlock(mesh);

	int x, y, z;
	for(x = 0; x < grid->mx; x++)
		for(y = 0; y < grid->my; y++)
			for(z = 0; z < grid->mz; z++)
	{
		int val = c_grid_get(grid, x, y, z);
		if(val & 0x2)
		{
			entity_t box = entity_new({
				c_name_new("box");
				c_movable_new(val);
				c_side_new(root, !(val&1), 1);
				c_model_new(mesh,
					(val&1) ? sauces("stone3.mat") : sauces("stone4.mat"), 1, 1);
			});
			/* c_model(&box)->before_draw = (before_draw_cb)cmd_model_before_draw; */
			c_spatial_set_pos(c_spatial(&box), vec3(x, y, z));
			c_node_add(c_node(&root), 1, box);
		}
	}

	/* getc(fd); */

	entity_signal(level->grid, sig("grid_update"), NULL, NULL);

	return level->grid;
}

static entity_t cmd_key(entity_t root, int argc, const char **argv)
{
	float x, y, z;
	int rotX, rotY, rotZ;
	int id, side;

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%d", &rotX);
	sscanf(argv[5], "%d", &rotY);
	sscanf(argv[6], "%d", &rotZ);
	sscanf(argv[7], "%d", &id);

	c_level_t *scene = c_level(&root);

	side = c_grid_get(c_grid(&scene->grid), x, y, z) & 1;

	entity_t key = entity_new({
		c_name_new("key");
		c_model_new(sauces("key.obj"), sauces("key.mat"), 0, 1);
		c_side_new(root, side, 0);
		c_key_new(rotX, rotY, rotZ, id);
	});

	/* sauces_mat("key")->transparency.color = vec4(1.0f, 1.0f, 1.0f, 0.4f); */
	/* c_model(&key)->before_draw = (before_draw_cb)cmd_model_before_draw; */

	c_spatial_set_pos(c_spatial(&key), vec3(x, y, z));

	return key;
}

static c_t *get_char(void)
{
	return ct_get_nth(ecm_get(ct_character), 0);
}

static float get_motion_power(pass_t *pass, c_menu_t *menu)
{
	return menu->motion_power;
}

static float get_ssao_power(pass_t *pass, c_menu_t *menu)
{
	return menu->ssao_power;
}

static float get_ssr_power(pass_t *pass, c_menu_t *menu)
{
	return menu->ssr_power;
}

static renderer_t *shift_renderer(c_menu_t *menu)
{
	renderer_t *self = renderer_new(1.0f);

	texture_t *query_mips = texture_new_2D(0, 0, 0, 5,
		buffer_new("depth",		true, -1),
		buffer_new("tiles0",	false, 4),
		buffer_new("tiles1",	false, 4),
		buffer_new("tiles2",	false, 4),
		buffer_new("tiles3",	false, 4));

	texture_t *gbuffer, *gbuffer2, *portal, *ssao, *light, *refr, *bloom,
			  *tmp, *final, *volum;

	gbuffer = texture_new_2D(0, 0, 0, 5,
		buffer_new("depth",     true, -1),
		buffer_new("albedo",    false,  4),
		buffer_new("nn",        false,  2),
		buffer_new("mr",        false, 2),
		buffer_new("emissive",  false, 3));

	gbuffer2 = texture_new_2D(0, 0, 0, 5,
		buffer_new("depth",	    true, -1),
		buffer_new("albedo",    false, 4),
		buffer_new("nn",        false,  2),
		buffer_new("mr",        false, 2),
		buffer_new("emissive",  false, 3));

	portal = texture_new_2D(0, 0, 0, 2,
		buffer_new("depth",	 true, -1),
		buffer_new("ignore", true, 1));

	light = texture_new_2D(0, 0, TEX_INTERPOLATE, 1,
			buffer_new("color",	true, 4));

	refr = texture_new_2D(0, 0, TEX_MIPMAP | TEX_INTERPOLATE, 1,
			buffer_new("color", false, 4));

	bloom = texture_new_2D(0, 0, TEX_MIPMAP | TEX_INTERPOLATE, 1,
			buffer_new("color", false, 4));

	tmp = texture_new_2D(0, 0, TEX_MIPMAP | TEX_INTERPOLATE, 1,
			buffer_new("color", false, 4));

	final = texture_new_2D(0, 0, TEX_MIPMAP | TEX_INTERPOLATE, 1,
		buffer_new("color",	true, 4));

	ssao = texture_new_2D(0, 0, TEX_INTERPOLATE, 1,
		buffer_new("occlusion", false, 1));

	volum =	texture_new_2D(0, 0, TEX_INTERPOLATE, 1,
		buffer_new("color",	false, 4));

	renderer_add_tex(self, "query_mips", 0.1f, query_mips);
	renderer_add_tex(self, "portal",	 1.0f, portal);
	renderer_add_tex(self, "tmp",		 1.0f, tmp);
	renderer_add_tex(self, "refr",		 1.0f, refr);
	renderer_add_tex(self, "bloom",		 1.0f, bloom);
	renderer_add_tex(self, "final",		 1.0f, final);
	renderer_add_tex(self, "ssao",		 0.5f, ssao);
	renderer_add_tex(self, "light",		 1.0f, light);
	renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);
	renderer_add_tex(self, "gbuffer2",	 1.0f, gbuffer2);
	renderer_add_tex(self, "volum",		 0.5f, volum);

	renderer_add_pass(self, "query_mips", "candle:query_mips", ref("visible"), 0,
			query_mips, query_mips, 0, ~0, 3,
			opt_clear_depth(1.0f, NULL),
			opt_clear_color(Z4, NULL),
			opt_skip(16)
	);
	renderer_add_pass(self, "query_mips", "candle:query_mips", ref("decals"), 0,
			query_mips, NULL, 0, ~0, 1,
			opt_skip(16)
	);
	renderer_add_pass(self, "query_mips", "candle:query_mips", ref("transparent"), 0,
			query_mips, query_mips, 0, ~0, 1,
			opt_skip(16)
	);
	renderer_add_pass(self, "svt", NULL, -1, 0,
			query_mips, query_mips, 0, ~0, 2,
			opt_callback((getter_cb)pass_process_query_mips),
			opt_skip(16)
	);

	renderer_add_pass(self, "gbuffer", "candle:gbuffer", ref("visible"), 0,
			gbuffer, gbuffer, 0, ~0, 2,
			opt_clear_depth(1.0f, NULL),
			opt_clear_color(Z4, NULL)
	);

	/* DECAL PASS */
	renderer_add_pass(self, "decals_pass", "candle:gbuffer", ref("decals"), BLEND,
			gbuffer, NULL, 0, ~0, 1,
			opt_tex("gbuffer", gbuffer, NULL)
	);

	/* renderer_add_pass(self, "portal", "portal", ref("portal"), */
	/* 		DEPTH_DISABLE, portal, portal, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{CLEAR_DEPTH, .number = 0.0f}, */
	/* 		{CLEAR_COLOR, .vec4 = vec4(0.0f)}, */
	/* 		{TEX, "gbuffer", .buffer = gbuffer}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	/* renderer_add_pass(self, "gbuffer2", "masked_gbuffer", ref("next_level"), 0, */
	/* 		gbuffer2, gbuffer2, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{CLEAR_DEPTH, .number = 1.0f}, */
	/* 		{CLEAR_COLOR, .vec4 = vec4(0.0f)}, */
	/* 		{CAM, .integer = 1}, */
	/* 		{TEX, "portal", .buffer = portal}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */
	/* renderer_add_pass(self, "copy_gbuffer", "copy_gbuffer", ref("quad"), */
	/* 		DEPTH_DISABLE, gbuffer, gbuffer, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "buf", .buffer = gbuffer2}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	renderer_add_pass(self, "ambient_light_pass", "candle:pbr", ref("ambient"),
			ADD, light, NULL, 0, ~0, 2,
			opt_clear_color(Z4, NULL),
			opt_tex("gbuffer", gbuffer, NULL)
	);

	renderer_add_pass(self, "render_pass", "candle:pbr", ref("light"),
			ADD, light, NULL, 0, ~0, 1,
			opt_tex("gbuffer", gbuffer, NULL)
	);

	/* renderer_add_pass(self, "volum_pass", "candle:volum", ref("light"), */
	/* 		ADD | CULL_DISABLE, volum, NULL, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "gbuffer", .buffer = gbuffer}, */
	/* 		{CLEAR_COLOR, .vec4 = vec4(0.0f)}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	/* renderer_add_pass(self, "render_pass2", "candle:pbr", ref("next_level_light"), */
	/* 		ADD, light, NULL, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "gbuffer", .buffer = gbuffer2}, */
	/* 		{CAM, .integer = 1}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	renderer_add_pass(self, "refraction", "candle:copy", ref("quad"), 0,
			refr, NULL, 0, ~0, 2,
			opt_tex("buf", light, NULL),
			opt_int("level", 0, NULL)
	);
	renderer_add_kawase(self, refr, tmp, 0, 1);
	renderer_add_kawase(self, refr, tmp, 1, 2);
	renderer_add_kawase(self, refr, tmp, 2, 3);

	renderer_add_pass(self, "transp_1", "candle:gbuffer", ref("transparent"),
			0, gbuffer, gbuffer, 0, ~0, 0);

	renderer_add_pass(self, "transp", "candle:transparent", ref("transparent"),
			DEPTH_EQUAL | DEPTH_LOCK, light, gbuffer, 0, ~0, 1,
			opt_tex("refr", refr, NULL)
	);


	renderer_add_pass(self, "ssao_pass", "candle:ssao", ref("quad"), 0,
			ssao, NULL, 0, ~0, 2,
			opt_tex("gbuffer", gbuffer, NULL),
			opt_clear_color(Z4, NULL)
	);

	renderer_add_pass(self, "final", "candle:final", ref("quad"), 0, final,
			NULL, 0, ~0, 8,
			opt_tex("gbuffer", gbuffer, NULL),
			opt_tex("light", light, NULL),
			opt_tex("refr", refr, NULL),
			opt_num("ssr_power", 0.0f, (getter_cb)get_ssr_power),
			opt_tex("ssao", ssao, NULL),
			opt_num("ssao_power", 0, (getter_cb)get_ssao_power),
			opt_tex("volum", volum, NULL),
			opt_usrptr(menu)
	);

	renderer_add_pass(self, "motion blur", "candle:motion", ref("quad"), 0,
			tmp, NULL, 0, ~0, 4,
			opt_tex("gbuffer", gbuffer, NULL),
			opt_tex("buf", final, NULL),
			opt_num("power", 0.0f, (getter_cb)get_motion_power),
			opt_usrptr(menu)
	);
	renderer_add_pass(self, "output_motion", "candle:copy", ref("quad"), 0,
			final, NULL, 0, ~0, 2,
			opt_tex("buf", tmp, NULL),
			opt_int("level", 0, NULL)
	);

	renderer_add_pass(self, "bloom_0", "candle:bright", ref("quad"), 0,
			bloom, NULL, 0, ~0, 1,
			opt_tex("buf", final, NULL)
	);
	renderer_add_kawase(self, bloom, tmp, 0, 1);
	renderer_add_kawase(self, bloom, tmp, 1, 2);
	renderer_add_kawase(self, bloom, tmp, 2, 3);

	renderer_add_pass(self, "bloom_1", "candle:upsample", ref("quad"), ADD,
			final, NULL, 0, ~0, 3,
			opt_tex("buf", bloom, NULL),
			opt_int("level", 3, NULL),
			opt_num("alpha", 0.5, NULL)
	);

	renderer_add_pass(self, "luminance_calc", NULL, -1, 0,
			final, NULL, 0, ~0, 3,
			opt_callback((getter_cb)pass_process_brightness),
			opt_skip(8),
			opt_usrptr(menu)
	);

	self->output = final;

	return self;
}

static entity_t cmd_spawn(entity_t root, int argc, const char **argv)
{
	vec3_t pos;
	int dir, side;

	sscanf(argv[1], "%f", &pos.x);
	sscanf(argv[2], "%f", &pos.y);
	sscanf(argv[3], "%f", &pos.z);
	sscanf(argv[4], "%d", &dir);

	c_level_t *level = c_level(&root);

	side = c_grid_get(c_grid(&level->grid), _vec3(pos));

	int side_dir = (side & 1) ? 1 : -1;
	level->spawn = entity_new({
		c_side_new(root, side, 0);
		c_spatial_new();
		c_name_new("spawn");
	});
	pos = vec3(pos.x, pos.y + 0.48 * side_dir, pos.z);
	c_spatial_t *spawnsc = c_spatial(&level->spawn);
	c_spatial_lock(spawnsc);
	c_spatial_set_pos(spawnsc, pos);

	dir++;
	while(dir > 0)
	{
		c_spatial_rotate_Y(spawnsc, -M_PI / 2.0f);
		dir--;
	}
	c_spatial_unlock(spawnsc);

	if(!get_char())
	{
		entity_t g = entity_new({
			c_name_new("gravity");
			c_force_new(0.0, 23 * side_dir, 0.0, 1);
		});

		entity_t body = entity_new({
			c_name_new("body");
		});

	
		entity_t character = entity_new({
			c_name_new("character");
			c_character_new(body, 1, g);
			c_side_new(root, side, 1);
			c_speaker_new();
			c_menu_new();
		});
		renderer_t *renderer = shift_renderer(c_menu(&character));	

		c_spatial_t *sc = c_spatial(&character);
		c_spatial_lock(sc);

		c_rigid_body(&character)->offset = -0.8f * side_dir;

		entity_t camera = entity_new({
			c_name_new("camera");
			c_camera_new(70, 0.1, 1000.0, 1, 1, 1, renderer);
			c_charlook_new(body, 1.9);
			c_side_new(root, side, 0);
		});
		if (c_openxr(&SYS)) {
			c_openxr(&SYS)->renderer = renderer;
			c_camera(&camera)->active = false;
		}
		c_charlook_reset(c_charlook(&camera));

		c_node_add(c_node(&character), 1, body);
		c_node_add(c_node(&body), 1, camera);


		level->pov = camera;

		c_spatial_set_model(sc, spawnsc->model_matrix);

		c_spatial_unlock(sc);
		level->mirror = entity_new({
			c_name_new("mirror");
			c_mirror_new(camera);
			c_camera_new(70, 0.1, 1000.0, 0, 0, 0, renderer);
		});
		c_camera(&level->mirror)->auto_transform = 0;

	}
	else
	{
		level->pov = level->spawn;
	}

	return level->spawn;
}

static entity_t cmd_bridge(entity_t root, int argc, const char **argv)
{
#define order(N1, N2, n1, n2) ((n1 < n2) ? \
		(N1 = n1, N2 = n2) : \
		(N1 = n2, N2 = n1))

	float x1, y1, z1, x2, y2, z2, cx, cy, cz;
	int key;

	sscanf(argv[1], "%f", &x1);
	sscanf(argv[2], "%f", &y1);
	sscanf(argv[3], "%f", &z1);
	sscanf(argv[4], "%f", &x2);
	sscanf(argv[5], "%f", &y2);
	sscanf(argv[6], "%f", &z2);
	sscanf(argv[7], "%f", &cx);
	sscanf(argv[8], "%f", &cy);
	sscanf(argv[9], "%f", &cz);
	sscanf(argv[10], "%d", &key);

	entity_t bridge = entity_new({
		c_name_new("bridge");
		c_side_new(root, -1, 1);
		c_bridge_new();
	});
	c_bridge_t *p = c_bridge(&bridge);

	order(p->ix1, p->ix2, x1, x2);
	order(p->iy1, p->iy2, y1, y2);
	order(p->iz1, p->iz2, z1, z2);

	p->ix1 -= cx; p->ix2 -= cx;
	p->iy1 -= cy; p->iy2 -= cy;
	p->iz1 -= cz; p->iz2 -= cz;

	p->min.x = p->ix1; p->min.y = p->iy1; p->min.z = p->iz1;
	p->max.x = p->ix2; p->max.y = p->iy2; p->max.z = p->iz2;

	p->min = vec3_sub(p->min, vec3(0.5f, 0.5f, 0.5f));
	p->max = vec3_add(p->max, vec3(0.5f, 0.5f, 0.5f));

	p->cx=cx;
	p->cy=cy;
	p->cz=cz;

	p->key=key;

	/* mesh_t *mesh = mesh_cuboid(0.5, */
			/* p->x1 - 0.01, p->y1 - 0.01, p->z1 - 0.01, */
			/* p->x2 + 0.01, p->y2 + 0.01, p->z2 + 0.01); */

	c_spatial_set_pos(c_spatial(&bridge), vec3(p->cx, p->cy, p->cz));
	c_bridge_ready(p);

	/* c_model(&bridge)->before_draw = (before_draw_cb)cmd_model_before_draw; */

	return bridge;
}

static entity_t cmd_door(entity_t root, int argc, const char **argv)
{
	float x, y, z;
	char next[256];
	int dir;
	int side;

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%d", &dir);
	strcpy(next, argv[5]); 
	/* TODO fix this in canle */
	char *last = &next[strlen(next) - 1];
	if(*last == '\n') *last = '\0';

	c_level_t *level = c_level(&root);

	side = c_grid_get(c_grid(&level->grid), x, y, z) & 1;

	mesh_t *door_mesh = sauces("door.obj");
	door_mesh->smooth_angle = 0;
	level->door = entity_new({
		c_name_new("door");
		c_model_new(door_mesh, sauces("door.mat"), 1, 1);
		c_side_new(root, side, 0);
		c_door_new(next);
	});

	/* c_model(&level->door)->before_draw = (before_draw_cb)cmd_model_before_draw; */

	c_spatial_set_pos(c_spatial(&level->door), vec3(x, y - 0.49 * (side ? -1:1), z));
	c_spatial_rotate_Y(c_spatial(&level->door), dir * M_PI / 2);
	c_spatial_rotate_X(c_spatial(&level->door), side ? -M_PI : 0);

	return level->door;
}

static entity_t cmd_light(entity_t root, int argc, const char **argv)
{
	float x, y, z, intensity;
	int side;
	vec4_t color = vec4(0,0,0,0.8);

	sscanf(argv[1], "%f", &x);
	sscanf(argv[2], "%f", &y);
	sscanf(argv[3], "%f", &z);
	sscanf(argv[4], "%f", &intensity);
	sscanf(argv[5], "%f", &color.x);
	sscanf(argv[6], "%f", &color.y);
	sscanf(argv[7], "%f", &color.z);

	color.x *= 0.8;
	color.y *= 0.8;
	color.z *= 0.8;

	c_level_t *level = c_level(&root);

	side = c_grid_get(c_grid(&level->grid), x, y, z);

	entity_t light = entity_new({
		c_name_new("light");
		c_side_new(root, side, 0);
		c_side_follow_new();
		c_light_new(20.0f, color);
	});

	c_spatial_set_pos(c_spatial(&light), vec3(x, y, z));

	return light;
}

void reg_custom_cmds()
{
	candle_reg_cmd("light", (cmd_cb)cmd_light);
	candle_reg_cmd("door", (cmd_cb)cmd_door);
	candle_reg_cmd("bridge", (cmd_cb)cmd_bridge);
	candle_reg_cmd("spawn", (cmd_cb)cmd_spawn);
	candle_reg_cmd("key", (cmd_cb)cmd_key);
	candle_reg_cmd("shift_grid", (cmd_cb)cmd_shift_grid);
}
