#include <candle.h>

#include "components/physics.h"
#include "components/bridge.h"
#include "components/character.h"
#include "components/charlook.h"
#include "components/door.h"
#include "components/grid.h"
#include "components/key.h"
#include "components/state.h"
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

	texture_t *gbuffer, *portal, *ssao, *light, *refr, *refr2, *selectable,
			  *final;

	if(!original)
	{
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

		refr = texture_new_2D(0, 0, TEX_MIPMAP,
			buffer_new("color",	1, 4));

		refr2 = texture_new_2D(0, 0, TEX_MIPMAP,
			buffer_new("color",	1, 4));


		renderer_add_tex(self, "portal",	 1.0f, portal);
		renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);
		renderer_add_tex(self, "light",	 1.0f, light);
		renderer_add_tex(self, "selectable", 1.0f, selectable);
		renderer_add_tex(self, "refr",		 1.0f, refr);
		renderer_add_tex(self, "refr2",		 1.0f, refr2);
		self->output = gbuffer;
	}
	else
	{
		gbuffer =	texture_new_2D(0, 0, 0, 0,
			buffer_new("nmr",	 1, 4),
			buffer_new("albedo", 1, 4),
			buffer_new("depth",	 1, -1)
		);
		final = texture_new_2D(0, 0, TEX_INTERPOLATE | TEX_MIPMAP,
			buffer_new("color",	1, 4));

		ssao = texture_new_2D(0, 0, 0,
			buffer_new("occlusion",	1, 1));

		light = texture_new_2D(0, 0, 0,
			buffer_new("color",	1, 4));

		final->track_brightness = 1;

		renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);
		renderer_add_tex(self, "final",		 1.0f, final);
		renderer_add_tex(self, "ssao",		 1.0f, ssao);
		renderer_add_tex(self, "light",	 1.0f, light);

		portal = renderer_tex(original, ref("portal"));
		refr = renderer_tex(original, ref("refr"));
		refr2 = renderer_tex(original, ref("refr2"));
		selectable = renderer_tex(original, ref("selectable"));

		self->output = final;
	}

	if(!original)
	{
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
		renderer_add_pass(self, "gbuffer", "masked_gbuffer", ref("visible"), 0,
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

	renderer_add_pass(self, "render_pass", "phong", ref("light"),
			DEPTH_LOCK | ADD | DEPTH_EQUAL | DEPTH_GREATER, light, gbuffer, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{NONE}
		}
	);

	renderer_add_pass(self, "refraction", "copy", ref("quad"), MANUAL_MIP,
			refr, NULL, 0,
		(bind_t[]){
			{TEX, "buf", .buffer = light},
			{INT, "level", .integer = 0},
			{NONE}
		}
	);

	if(!original)
	{
		renderer_add_kawase(self, refr, refr2, 1);
		renderer_add_kawase(self, refr, refr2, 2);
		renderer_add_kawase(self, refr, refr2, 3);

		renderer_add_pass(self, "transp", "transparency", ref("transparent"),
				DEPTH_EQUAL, light, gbuffer, 0,
			(bind_t[]){
				{TEX, "refr", .buffer = refr},
				{NONE}
			}
		);
		renderer_add_pass(self, "transp_1", "gbuffer", ref("transparent"),
				0, gbuffer, gbuffer, 0, (bind_t[]){ {NONE} });
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

	char open_map_name[256];
	entity_t camera, g, body, character;

	entity_add_component(SYS, c_physics_new());
	entity_add_component(SYS, (c_t*)c_editmode_new());
	/* c_editmode_activate(c_editmode(&SYS)); */

	c_sauces_index_dir(c_sauces(&SYS), "resauces");

	reg_custom_cmds();

	g = entity_new(c_name_new("gravity"), c_force_new(0.0, -23, 0.0, 1));

	body = entity_new(c_name_new("body"), c_node_new());


	renderer_t *renderer = shift_renderer(NULL);
	
	camera = entity_new(
			c_name_new("camera"),
			c_camera_new(70, 0.1, 100.0, 0, 1, 0, renderer),
			c_charlook_new(body, 1.9)
	);

	character = entity_new(
			c_name_new("character"),
			c_character_new(body, 1, g)
	);

	c_spacial_set_pos(c_spacial(&camera), vec3(0.0, 0.7, 0.0));
	c_node_add(c_node(&character), 1, body);
	c_node_add(c_node(&body), 1, camera);

	sprintf(open_map_name, "resauces/maps/%s.xmap", argc > 1 ? argv[1] : "0");

	entity_add_component(SYS, c_state_new(open_map_name));

	entity_new(c_name_new("ambient"), c_light_new(-1.0f,
				vec4(1.0f, 1.0f, 1.0f, 0.1f), 0));

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
	c_spacial_set_scale(c_spacial(&venus), vec3(1.69, 1.0, 1));

	mat_t *bok = mat_new("bk");
	bok->albedo.texture = sauces("boku.png");
	bok->albedo.blend = 1;

	entity_t boku = entity_new(
			c_name_new("venus"),
			c_model_new(glass, bok, 1, 1)
	);
	c_spacial_set_pos(c_spacial(&boku), vec3(7.0, 6.5, -0.4));
	c_spacial_set_scale(c_spacial(&boku), vec3(1.69, 1.0, 1));

	//c_window_toggle_fullscreen(c_window(&candle->systems));

	candle_wait();

	printf("Exiting.\n");

	return 0;
}
//*/
