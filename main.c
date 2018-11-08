#include <candle.h>

#include "components/physics.h"
#include "components/bridge.h"
#include "components/character.h"
#include "components/charlook.h"
#include "components/door.h"
#include "components/grid.h"
#include "components/key.h"
#include "components/level.h"
#include "components/movable.h"
#include "components/side.h"
#include "components/side_follow.h"
#include "components/mirror.h"

#include <components/node.h>
#include <components/model.h>
#include <components/force.h>
#include <components/name.h>
#include <components/camera.h>
#include <components/ambient.h>
#include <components/decal.h>
#include <components/sprite.h>
#include <components/light.h>
#include <systems/editmode.h>

#include "cmds.h"

#include <stdio.h>

/* static mesh_t *ico; */
/* void spawn_sprite(entity_t par) */
/* { */
/* 	float x = ((float)(rand()%120)) / 10.0f; */
/* 	float y = ((float)(rand()%120)) / 24.0f + 6.0f; */
/* 	float z = ((float)(rand()%120)) / 10.0f; */
/* 	float s = ((float)(rand()%98)) / 100.0f + 0.02f; */
/* 	entity_t sprite = entity_new(c_model_new(ico, NULL, 0, 1)); */
/* 	c_spacial_t *sc = c_spacial(&sprite); */
/* 	c_spacial_lock(sc); */
/* 	c_spacial_set_pos(sc, vec3(x, y, z)); */
/* 	c_spacial_set_scale(sc, vec3(s)); */
/* 	c_spacial_unlock(sc); */

/* 	c_node_add(c_node(&par), 1, sprite); */
/* } */

renderer_t *shift_renderer(renderer_t *original)
{
	renderer_t *self = renderer_new(0.8f);

	texture_t *gbuffer, *portal, *ssao, *light, *refr, *tmp, *selectable,
			  *final;
	uint32_t light_group;

	if(!original)
	{
		light_group = ref("light");
		gbuffer =	texture_new_2D(0, 0, 0, 0,
			buffer_new("nmr",	 1, 4),
			buffer_new("albedo", 1, 4),
			buffer_new("depth",	 1, -1));

		portal = texture_new_2D(0, 0, 0,
			buffer_new("ignore", 1, 1),
			buffer_new("depth",	 1, -1));

		light = texture_new_2D(0, 0, 0,
			buffer_new("color",	1, 4));

		selectable = texture_new_2D(0, 0, 0,
			buffer_new("geomid", 1, 2),
			buffer_new("id",	 1, 2),
			buffer_new("depth",	 1, -1));

		refr = texture_new_2D(0, 0, TEX_MIPMAP, buffer_new("color", 1, 4));

		tmp = texture_new_2D(0, 0, TEX_MIPMAP, buffer_new("color", 1, 4));


		renderer_add_tex(self, "portal",	 1.0f, portal);
		renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);
		renderer_add_tex(self, "light",		 1.0f, light);
		renderer_add_tex(self, "selectable", 1.0f, selectable);
		renderer_add_tex(self, "tmp",		 1.0f, tmp);
		renderer_add_tex(self, "refr",		 1.0f, refr);
		self->output = gbuffer;

		renderer_add_pass(self, "gbuffer", "gbuffer", ref("visible"), 0,
				gbuffer, gbuffer, 0,
			(bind_t[]){
				{CLEAR_DEPTH, .number = 1.0f},
				{CLEAR_COLOR, .vec4 = vec4(0.0f)},
				{NONE}
			}
		);

		renderer_add_pass(self, "selectable", "select", ref("selectable"),
				0, selectable, selectable, 0,
			(bind_t[]){
				{CLEAR_DEPTH, .number = 1.0f},
				{CLEAR_COLOR, .vec4 = vec4(0.0f)},
				{NONE}
			}
		);

		/* DECAL PASS */
		renderer_add_pass(self, "decals_pass", "decals", ref("decals"),
				DEPTH_LOCK | DEPTH_EQUAL | DEPTH_GREATER,
				gbuffer, gbuffer, 0,
			(bind_t[]){
				{TEX, "gbuffer", .buffer = gbuffer},
				{NONE}
			}
		);

	}
	else
	{
		light_group = ref("next_level_light");
		gbuffer =	texture_new_2D(0, 0, 0, 0,
			buffer_new("nmr",	 1, 4),
			buffer_new("albedo", 1, 4),
			buffer_new("depth",	 1, -1)
		);
		final = texture_new_2D(0, 0, TEX_INTERPOLATE | TEX_MIPMAP,
			buffer_new("color",	1, 4));

		ssao = texture_new_2D(0, 0, 0, buffer_new("occlusion",	1, 1));

		light = texture_new_2D(0, 0, 0, buffer_new("color",	1, 4));

		final->track_brightness = 1;

		renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);
		renderer_add_tex(self, "final",		 1.0f, final);
		renderer_add_tex(self, "ssao",		 1.0f, ssao);
		renderer_add_tex(self, "light",		 1.0f, light);

		refr = renderer_tex(original, ref("refr"));
		tmp = renderer_tex(original, ref("tmp"));
		portal = renderer_tex(original, ref("portal"));
		selectable = renderer_tex(original, ref("selectable"));

		self->output = final;

		renderer_add_pass(self, "gbuffer", "masked_gbuffer", ref("next_level"), 0,
				gbuffer, gbuffer, 0,
			(bind_t[]){
				{CLEAR_DEPTH, .number = 1.0f},
				{CLEAR_COLOR, .vec4 = vec4(0.0f)},
				{TEX, "portal", .buffer = portal},
				{NONE}
			}
		);
	}

	renderer_add_pass(self, "ambient_light_pass", "phong", ref("ambient"),
			ADD, light, NULL, 0,
		(bind_t[]){
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{TEX, "gbuffer", .buffer = gbuffer},
			{NONE}
		}
	);

	renderer_add_pass(self, "render_pass", "phong", light_group,
			DEPTH_LOCK | ADD | DEPTH_EQUAL | DEPTH_GREATER, light, gbuffer, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{NONE}
		}
	);

	if(!original)
	{
		renderer_add_pass(self, "refraction", "copy", ref("quad"), MANUAL_MIP,
				refr, NULL, 0,
			(bind_t[]){
				{TEX, "buf", .buffer = light},
				{INT, "level", .integer = 0},
				{NONE}
			}
		);

		renderer_add_kawase(self, refr, tmp, 0, 1);
		renderer_add_kawase(self, refr, tmp, 1, 2);
		renderer_add_kawase(self, refr, tmp, 2, 3);

		renderer_add_pass(self, "transp_1", "gbuffer", ref("transparent"),
				0, gbuffer, gbuffer, 0, (bind_t[]){ {NONE} });

		renderer_add_pass(self, "transp", "transparency", ref("transparent"),
				DEPTH_EQUAL | DEPTH_LOCK, light, gbuffer, 0,
			(bind_t[]){
				{TEX, "refr", .buffer = refr},
				{NONE}
			}
		);

		renderer_add_pass(self, "portal", "portal", ref("portal"),
				DEPTH_DISABLE, portal, portal, 0,
			(bind_t[]){
				{CLEAR_DEPTH, .number = 0.0f},
				{CLEAR_COLOR, .vec4 = vec4(0.0f)},
				{TEX, "gbuffer", .buffer = gbuffer},
				{NONE}
			}
		);
	}
	else
	{
		texture_t *gb = renderer_tex(original, ref("gbuffer"));
		texture_t *rn = renderer_tex(original, ref("light"));
		refr = renderer_tex(original, ref("refr"));
		tmp = renderer_tex(original, ref("tmp"));

		renderer_add_pass(self, "copy_gbuffer", "copy_gbuffer", ref("quad"),
				DEPTH_DISABLE, gb, gb, 0,
			(bind_t[]){
				{TEX, "buf", .buffer = gbuffer},
				{NONE}
			}
		);

		renderer_add_pass(self, "copy_light", "copy", ref("quad"), 0,
				rn, NULL, 0,
			(bind_t[]){
				{TEX, "buf", .buffer = light},
				{INT, "level", .integer = 0},
				{NONE}
			}
		);

		gbuffer = gb;
		light = rn;

		renderer_add_pass(self, "ssao_pass", "ssao", ref("quad"), 0,
				ssao, NULL, 0,
			(bind_t[]){
				{TEX, "gbuffer", .buffer = gbuffer},
				{NONE}
			}
		);

		renderer_add_pass(self, "final", "ssr", ref("quad"), 0, final, NULL, 0,
			(bind_t[]){
				{CLEAR_COLOR, .vec4 = vec4(0.0f)},
				{TEX, "gbuffer", .buffer = gbuffer},
				{TEX, "light", .buffer = light},
				{TEX, "refr", .buffer = refr},
				{TEX, "ssao", .buffer = ssao},
				{NONE}
			}
		);

		renderer_add_pass(self, "bloom_%d", "bright", ref("quad"), MANUAL_MIP,
				refr, NULL, 0,
			(bind_t[]){
				{TEX, "buf", .buffer = final},
				{NONE}
			}
		);
		renderer_add_kawase(self, refr, tmp, 0, 1);
		renderer_add_kawase(self, refr, tmp, 1, 2);
		renderer_add_kawase(self, refr, tmp, 2, 3);

		renderer_add_pass(self, "bloom", "upsample", ref("quad"), MANUAL_MIP | ADD,
				final, NULL, 0,
			(bind_t[]){
				{TEX, "buf", .buffer = refr},
				{INT, "level", .integer = 3},
				{NUM, "alpha", .number = 0.5},
				{NONE}
			}
		);

	}



	return self;
}

int main(int argc, char **argv)
{
	/* ico = mesh_new(); */
	/* mesh_lock(ico); */
	/* mesh_ico(ico, 0.1f); */
	/* mesh_select(ico, SEL_EDITING, MESH_FACE, -1); */
	/* mesh_subdivide(ico, 2); */
	/* mesh_spherize(ico, 1.0f); */
	/* mesh_unlock(ico); */


	entity_add_component(SYS, (c_t*)c_editmode_new());
	/* c_editmode_activate(c_editmode(&SYS)); */

	c_sauces_index_dir(c_sauces(&SYS), "resauces");

	reg_custom_cmds();

	const char *level = argc > 1 ? argv[1] : "0";
	entity_t lvl = entity_new(c_name_new(level), c_level_new(level, 0));
	/* entity_new(c_name_new(level), c_level_new(level, 1)); */

	entity_new(c_name_new("ambient"), c_light_new(-1.0f,
				vec4(1.0f, 1.0f, 1.0f, 0.3f), 0));
	c_level_set_active(c_level(&lvl), 1);
	entity_add_component(SYS, c_physics_new());

	/* mat_t *mat = mat_new("tt"); */
	/* entity_t decal = entity_new(c_decal_new(mat, 1)); */
	/* c_spacial_set_pos(c_spacial(&decal), vec3(7.0, 6.5, -0.5)); */
	/* c_spacial_rotate_Y(c_spacial(&decal), M_PI / 5); */
	/* c_spacial_rotate_X(c_spacial(&decal), -M_PI / 2); */
	/* mat->albedo.texture = c_camera(&camera)->renderer->output; */
	/* mat->metalness.color = vec4(0); */
	/* mat->roughness.color = vec4(0); */
	/* mat->albedo.blend = 1; */

	/* entity_t par = entity_new(c_node_new()); */
	/* for(int i = 0; i < 3000; i++) */
	/* { */
	/* 	spawn_sprite(par); */
	/* } */
	mat_t *mat = mat_new("gl");
	mat->roughness.texture = sauces("rough.png");
	mat->roughness.blend = 0.4;
	mat->transparency.color = vec4(0, 0, 0, 1);
	mesh_t *glass = mesh_new();
	mesh_quad(glass);
	entity_t venus = entity_new(
			c_name_new("venus"),
			c_model_new(glass, mat, 1, 1)
	);
	c_spacial_set_pos(c_spacial(&venus), vec3(7.0, 6.5, -0.37));
	c_spacial_set_scale(c_spacial(&venus), vec3(1.69 * 0.3, 1.0 * 0.3, 1));

	mat_t *bok = mat_new("bk");
	bok->albedo.texture = sauces("boku.png");
	bok->albedo.blend = 1;

	entity_t boku = entity_new(
			c_name_new("venus"),
			c_model_new(glass, bok, 1, 1)
	);
	c_spacial_set_pos(c_spacial(&boku), vec3(7.0, 6.5, -0.4));
	c_spacial_set_scale(c_spacial(&boku), vec3(1.69 * 0.3, 1.0 * 0.3, 1));

	//c_window_toggle_fullscreen(c_window(&candle->systems));

	candle_wait();

	printf("Exiting.\n");

	return 0;
}
//*/
