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
#include "components/rigid_body.h"

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

#include "openal.candle/openal.h"
#include "openal.candle/speaker.h"

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
/* 	c_spatial_t *sc = c_spatial(&sprite); */
/* 	c_spatial_lock(sc); */
/* 	c_spatial_set_pos(sc, vec3(x, y, z)); */
/* 	c_spatial_set_scale(sc, vec3(s)); */
/* 	c_spatial_unlock(sc); */

/* 	c_node_add(c_node(&par), 1, sprite); */
/* } */

renderer_t *debug_renderer()
{
	renderer_t *self = renderer_new(0.66f);

	_texture_new_2D_pre(0, 0, 0);
		buffer_new("depth",		true, -1);
		buffer_new("tiles0",	false, 4);
		buffer_new("tiles1",	false, 4);
		buffer_new("tiles2",	false, 4);
		buffer_new("tiles3",	false, 4);
	texture_t *query_mips = _texture_new(0);

	_texture_new_2D_pre(0, 0, 0);
		buffer_new("depth",	 	true, -1);
		buffer_new("albedo", 	false, 4);
		buffer_new("nmr",	 	true, 4);
		buffer_new("emissive",	true, 3);
	texture_t *gbuffer = _texture_new(0);
	renderer_add_tex(self, "query_mips", 0.1f, query_mips);
	renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);

	renderer_add_pass(self, "query_mips", "query_mips", ref("visible"), 0,
			query_mips, query_mips, 0,
		(bind_t[]){
			{CLEAR_DEPTH, .number = 1.0f},
			{SKIP, .integer = 2048},
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{NONE}
		}
	);


	renderer_add_pass(self, "svt", NULL, -1, 0,
			query_mips, query_mips, 0,
		(bind_t[]){
			{CALLBACK, .getter = (getter_cb)renderer_process_query_mips, .usrptr = self},
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{SKIP, .integer = 2048},
			{NONE}
		}
	);
	renderer_add_pass(self, "gbuffer", "gbuffer", ref("visible"), 0,
			gbuffer, gbuffer, 0,
		(bind_t[]){
			{CLEAR_DEPTH, .number = 1.0f},
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{NONE}
		}
	);
	self->output = gbuffer;

	return self;
}

renderer_t *shift_renderer()
{
	/* renderer_t *self = renderer_new(1.0f); */
	renderer_t *self = renderer_new(0.66f);

	_texture_new_2D_pre(0, 0, 0);
		buffer_new("depth",		true, -1);
		buffer_new("tiles0",	false, 4);
		buffer_new("tiles1",	false, 4);
		buffer_new("tiles2",	false, 4);
		buffer_new("tiles3",	false, 4);
	texture_t *query_mips = _texture_new(0);

	texture_t *gbuffer, *gbuffer2, *portal, *ssao, *light, *refr,
			  *tmp, *final;

	_texture_new_2D_pre(0, 0, 0);
		buffer_new("depth",	 	true, -1);
		buffer_new("albedo", 	true, 4);
		buffer_new("nmr",	 	true, 4);
		buffer_new("emissive",	false, 3);
	gbuffer = _texture_new(0);

	_texture_new_2D_pre(0, 0, 0);
		buffer_new("depth",	 true, -1);
		buffer_new("albedo", true, 4);
		buffer_new("nmr",	 false, 4);
	gbuffer2 = _texture_new(0);

	_texture_new_2D_pre(0, 0, 0);
		buffer_new("depth",	 true, -1);
		buffer_new("ignore", true, 1);
	portal = _texture_new(0);

	light = texture_new_2D(0, 0, TEX_INTERPOLATE,
			buffer_new("color",	true, 4));

	refr = texture_new_2D(0, 0, TEX_MIPMAP | TEX_INTERPOLATE,
			buffer_new("color", true, 4));

	tmp = texture_new_2D(0, 0, TEX_MIPMAP | TEX_INTERPOLATE,
			buffer_new("color", true, 4));

	final = texture_new_2D(0, 0, TEX_INTERPOLATE | TEX_MIPMAP,
		buffer_new("color",	true, 4));

	ssao = texture_new_2D(0, 0, 0, buffer_new("occlusion",	true, 1));
	texture_t *volum =	texture_new_2D(0, 0, TEX_INTERPOLATE,
		buffer_new("color",	false, 4)
	);

	renderer_add_tex(self, "query_mips", 0.1f, query_mips);
	renderer_add_tex(self, "portal",	 1.0f, portal);
	renderer_add_tex(self, "tmp",		 1.0f, tmp);
	renderer_add_tex(self, "refr",		 1.0f, refr);
	renderer_add_tex(self, "final",		 1.0f, final);
	renderer_add_tex(self, "ssao",		 1.0f, ssao);
	renderer_add_tex(self, "light",		 1.0f, light);
	renderer_add_tex(self, "gbuffer",	 1.0f, gbuffer);
	renderer_add_tex(self, "gbuffer2",	 1.0f, gbuffer2);
	renderer_add_tex(self, "volum",		 0.6f, volum);

	renderer_add_pass(self, "query_mips", "query_mips", ref("visible"), 0,
			query_mips, query_mips, 0,
		(bind_t[]){
			{CLEAR_DEPTH, .number = 1.0f},
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{NONE}
		}
	);
	renderer_add_pass(self, "query_mips", "query_mips", ref("transparent"), 0,
			query_mips, query_mips, 0,
		(bind_t[]){
			{NONE}
		}
	);
	renderer_add_pass(self, "svt", NULL, -1, 0,
			query_mips, query_mips, 0,
		(bind_t[]){
			{CALLBACK, .getter = (getter_cb)renderer_process_query_mips, .usrptr = self},
			{NONE}
		}
	);

	renderer_add_pass(self, "gbuffer", "gbuffer", ref("visible"), 0,
			gbuffer, gbuffer, 0,
		(bind_t[]){
			{CLEAR_DEPTH, .number = 1.0f},
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{NONE}
		}
	);

	/* renderer_add_pass(self, "portal", "portal", ref("portal"), */
	/* 		DEPTH_DISABLE, portal, portal, 0, */
	/* 	(bind_t[]){ */
	/* 		{CLEAR_DEPTH, .number = 0.0f}, */
	/* 		{CLEAR_COLOR, .vec4 = vec4(0.0f)}, */
	/* 		{TEX, "gbuffer", .buffer = gbuffer}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	/* renderer_add_pass(self, "gbuffer2", "masked_gbuffer", ref("next_level"), 0, */
	/* 		gbuffer2, gbuffer2, 0, */
	/* 	(bind_t[]){ */
	/* 		{CLEAR_DEPTH, .number = 1.0f}, */
	/* 		{CLEAR_COLOR, .vec4 = vec4(0.0f)}, */
	/* 		{CAM, .integer = 1}, */
	/* 		{TEX, "portal", .buffer = portal}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */
	/* renderer_add_pass(self, "copy_gbuffer", "copy_gbuffer", ref("quad"), */
	/* 		DEPTH_DISABLE, gbuffer, gbuffer, 0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "buf", .buffer = gbuffer2}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	renderer_add_pass(self, "ambient_light_pass", "phong", ref("ambient"),
			ADD, light, NULL, 0,
		(bind_t[]){
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{TEX, "gbuffer", .buffer = gbuffer},
			{NONE}
		}
	);

	renderer_add_pass(self, "render_pass", "phong", ref("light"),
			ADD, light, NULL, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{NONE}
		}
	);

	renderer_add_pass(self, "volum_pass", "volum", ref("light"),
			ADD | CULL_DISABLE, volum, NULL, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{CLEAR_COLOR, .vec4 = vec4(0.0f)},
			{NONE}
		}
	);

	/* renderer_add_pass(self, "render_pass2", "phong", ref("next_level_light"), */
	/* 		ADD, light, NULL, 0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "gbuffer", .buffer = gbuffer2}, */
	/* 		{CAM, .integer = 1}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	renderer_add_pass(self, "refraction", "copy", ref("quad"), 0,
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

	renderer_add_pass(self, "transp", "gbuffer", ref("transparent"),
			DEPTH_EQUAL | DEPTH_LOCK, light, gbuffer, 0,
		(bind_t[]){
			{TEX, "refr", .buffer = refr},
			{NONE}
		}
	);

	renderer_add_pass(self, "ssao_pass", "ssao", ref("quad"), 0,
			ssao, NULL, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{NUM, "power", .number = 1.0},
			{NONE}
		}
	);

	renderer_add_pass(self, "final", "ssr", ref("quad"), 0, final,
			NULL, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{TEX, "light", .buffer = light},
			{TEX, "refr", .buffer = refr},
			{TEX, "ssao", .buffer = ssao},
			{TEX, "volum", .buffer = volum},
			{NONE}
		}
	);

	renderer_add_pass(self, "bloom_0", "bright", ref("quad"), 0,
			refr, NULL, 0,
		(bind_t[]){
			{TEX, "buf", .buffer = final},
			{NONE}
		}
	);
	renderer_add_kawase(self, refr, tmp, 0, 1);
	renderer_add_kawase(self, refr, tmp, 1, 2);
	renderer_add_kawase(self, refr, tmp, 2, 3);

	renderer_add_pass(self, "bloom_1", "upsample", ref("quad"), ADD,
			final, NULL, 0,
		(bind_t[]){
			{TEX, "buf", .buffer = refr},
			{INT, "level", .integer = 3},
			{NUM, "alpha", .number = 0.5},
			{NONE}
		}
	);

	renderer_add_pass(self, "motion blur", "motion", ref("quad"), TRACK_BRIGHT,
			tmp, NULL, 0,
		(bind_t[]){
			{TEX, "gbuffer", .buffer = gbuffer},
			{TEX, "buf", .buffer = final},
			{NONE}
		}
	);

	self->output = tmp;

	return self;
}

int main(int argc, char **argv)
{
	if (candle_utility(argc, argv))
		return 0;

	entity_add_component(SYS, (c_t*)c_editmode_new());
	entity_add_component(SYS, (c_t*)c_openal_new());
	/* c_editmode_activate(c_editmode(&SYS)); */

	c_sauces_index_dir(c_sauces(&SYS), "resauces");

	reg_custom_cmds();

	const char *level = argc > 1 ? argv[1] : "0";
	entity_t lvl = entity_new(c_name_new(level), c_level_new(level, 0));
	/* entity_new(c_name_new(level), c_level_new(level, 1)); */

	entity_new(c_name_new("ambient"), c_light_new(-1.0f,
				vec4(0.3f, 0.3f, 0.3f, 0.3f), 0));
	c_level_set_active(c_level(&lvl), 1);
	entity_add_component(SYS, c_physics_new());

	/* ico = mesh_new(); */
	/* mesh_lock(ico); */
	/* mesh_ico(ico, 0.1f); */
	/* mesh_select(ico, SEL_EDITING, MESH_FACE, -1); */
	/* mesh_subdivide(ico, 2); */
	/* mesh_spherize(ico, 1.0f); */
	/* mesh_unlock(ico); */

	/* entity_t par = entity_new(c_node_new()); */
	/* for(int i = 0; i < 3000; i++) */
	/* { */
	/* 	spawn_sprite(par); */
	/* } */

	//c_window_toggle_fullscreen(c_window(&candle->systems));

	candle_wait();

	printf("Exiting.\n");

	return 0;
}
//*/
