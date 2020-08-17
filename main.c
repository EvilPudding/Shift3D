#include "candle/candle.h"

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
#include "components/force.h"

#include "candle/components/node.h"
#include "candle/components/model.h"
#include "candle/components/name.h"
#include "candle/components/camera.h"
#include "candle/components/ambient.h"
#include "candle/components/decal.h"
#include "candle/components/sprite.h"
#include "candle/components/light.h"
#include "candle/systems/editmode.h"

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

float g_motion_power = 1.0f;
float g_ssao_power = 0.6f;
float g_ssr_power = 1.0f;
bool_t g_squints_active = true;

static float get_motion_power()
{
	return g_motion_power;
}

static float get_ssao_power()
{
	return g_ssao_power;
}

static float get_ssr_power()
{
	return g_ssr_power;
}

static texture_t *get_last_pass(renderer_t *renderer)
{
	if (g_motion_power > 0.05f)
	{
		return renderer_tex(renderer, ref("tmp"));
	}
	else
	{
		return renderer_tex(renderer, ref("final"));
	}
}



static renderer_t *shift_renderer()
{
	/* renderer_t *self = renderer_new(1.0f); */
	renderer_t *self = renderer_new(0.66f);

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

	renderer_add_tex(self, "query_mips", 0.05f, query_mips);
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

	renderer_add_pass(self, "query_mips", "query_mips", ref("visible"), 0,
			query_mips, query_mips, 0, ~0, 3,
			opt_clear_depth(1.0f, NULL),
			opt_clear_color(Z4, NULL),
			opt_skip(16)
	);
	renderer_add_pass(self, "query_mips", "query_mips", ref("decals"), 0,
			query_mips, NULL, 0, ~0, 1,
			opt_skip(16)
	);
	renderer_add_pass(self, "query_mips", "query_mips", ref("transparent"), 0,
			query_mips, query_mips, 0, ~0, 1,
			opt_skip(16)
	);
	renderer_add_pass(self, "svt", NULL, -1, 0,
			query_mips, query_mips, 0, ~0, 2,
			opt_callback((getter_cb)pass_process_query_mips),
			opt_skip(16)
	);

	renderer_add_pass(self, "gbuffer", "gbuffer", ref("visible"), 0,
			gbuffer, gbuffer, 0, ~0, 2,
			opt_clear_depth(1.0f, NULL),
			opt_clear_color(Z4, NULL)
	);

	/* DECAL PASS */
	renderer_add_pass(self, "decals_pass", "gbuffer", ref("decals"), BLEND,
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

	renderer_add_pass(self, "ambient_light_pass", "phong", ref("ambient"),
			ADD, light, NULL, 0, ~0, 2,
			opt_clear_color(Z4, NULL),
			opt_tex("gbuffer", gbuffer, NULL)
	);

	renderer_add_pass(self, "render_pass", "phong", ref("light"),
			ADD, light, NULL, 0, ~0, 1,
			opt_tex("gbuffer", gbuffer, NULL)
	);

	/* renderer_add_pass(self, "volum_pass", "volum", ref("light"), */
	/* 		ADD | CULL_DISABLE, volum, NULL, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "gbuffer", .buffer = gbuffer}, */
	/* 		{CLEAR_COLOR, .vec4 = vec4(0.0f)}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	/* renderer_add_pass(self, "render_pass2", "phong", ref("next_level_light"), */
	/* 		ADD, light, NULL, 0, ~0, */
	/* 	(bind_t[]){ */
	/* 		{TEX, "gbuffer", .buffer = gbuffer2}, */
	/* 		{CAM, .integer = 1}, */
	/* 		{NONE} */
	/* 	} */
	/* ); */

	renderer_add_pass(self, "refraction", "copy", ref("quad"), 0,
			refr, NULL, 0, ~0, 2,
			opt_tex("buf", light, NULL),
			opt_int("level", 0, NULL)
	);
	renderer_add_kawase(self, refr, tmp, 0, 1);
	renderer_add_kawase(self, refr, tmp, 1, 2);
	renderer_add_kawase(self, refr, tmp, 2, 3);

	renderer_add_pass(self, "transp_1", "gbuffer", ref("transparent"),
			0, gbuffer, gbuffer, 0, ~0, 0);

	renderer_add_pass(self, "transp", "transparency", ref("transparent"),
			DEPTH_EQUAL | DEPTH_LOCK, light, gbuffer, 0, ~0, 1,
			opt_tex("refr", refr, NULL)
	);


	renderer_add_pass(self, "ssao_pass", "ssao", ref("quad"), 0,
			ssao, NULL, 0, ~0, 2,
			opt_tex("gbuffer", gbuffer, NULL),
			opt_clear_color(Z4, NULL)
	);

	renderer_add_pass(self, "final", "ssr", ref("quad"), 0, final,
			NULL, 0, ~0, 7,
			opt_tex("gbuffer", gbuffer, NULL),
			opt_tex("light", light, NULL),
			opt_tex("refr", refr, NULL),
			opt_num("ssr_power", 0.0f, (getter_cb)get_ssr_power),
			opt_tex("ssao", ssao, NULL),
			opt_num("ssao_power", 0, (getter_cb)get_ssao_power),
			opt_tex("volum", volum, NULL)
	);

	renderer_add_pass(self, "motion blur", "motion", ref("quad"), 0,
			tmp, NULL, 0, ~0, 3,
			opt_tex("gbuffer", gbuffer, NULL),
			opt_tex("buf", final, NULL),
			opt_num("power", 0.0f, (getter_cb)get_motion_power)
	);
	renderer_add_pass(self, "output_motion", "copy", ref("quad"), 0,
			final, NULL, 0, ~0, 2,
			opt_tex("buf", tmp, NULL),
			opt_int("level", 0, NULL)
	);

	renderer_add_pass(self, "bloom_0", "bright", ref("quad"), 0,
			bloom, NULL, 0, ~0, 1,
			opt_tex("buf", final, NULL)
	);
	renderer_add_kawase(self, bloom, tmp, 0, 1);
	renderer_add_kawase(self, bloom, tmp, 1, 2);
	renderer_add_kawase(self, bloom, tmp, 2, 3);

	renderer_add_pass(self, "bloom_1", "upsample", ref("quad"), ADD,
			final, NULL, 0, ~0, 3,
			opt_tex("buf", bloom, NULL),
			opt_int("level", 3, NULL),
			opt_num("alpha", 0.5, NULL)
	);

	renderer_add_pass(self, "luminance_calc", NULL, -1, 0,
			final, NULL, 0, ~0, 2,
			opt_callback((getter_cb)pass_process_brightness),
			opt_skip(16)
	);

	self->output = final;

	return self;
}

int main(int argc, char **argv)
{
	/* for (int ri = 3; ri >= 1; ri--) */
	/* { */
	/* 	float r = ((float)ri) / 3.0f; */
	/* 	float offset = 2.0f * M_PI * ((float)rand()) / RAND_MAX; */
	/* 	float inc = (2.0f * M_PI / r) / 10.0f; */
	/* 	int steps = floorf((M_PI * 2.0f) / inc); */
	/* 	for (int ai = 0; ai < steps; ai++) */
	/* 	{ */
	/* 		float a = ((float)ai) * ((M_PI * 2.0f) / steps) + offset; */
	/* 		float offsetr = 0.0f; */
	/* 		/1* float offsetr = 0.1f * ((float)rand()) / RAND_MAX; *1/ */
	/* 		float x = cosf(a) * (r - offsetr); */
	/* 		float y = sinf(a) * (r - offsetr); */
	/* 		printf("%f, %f,   %f\n", x, y, sqrtf(x * x + y * y)); */
	/* 	} */
	/* } */
	/* exit(1); */
	candle_init(argv[0]);

	if (candle_utility(argc, argv))
		return 0;

	entity_add_component(SYS, (c_t*)c_editmode_new());
	entity_add_component(SYS, (c_t*)c_openal_new());
	/* c_editmode_activate(c_editmode(&SYS)); */

	reg_custom_cmds();

	const char *level = argc > 1 ? argv[1] : "0";
	entity_t lvl = entity_new({
		c_name_new(level);
		c_level_new(level, 0);
	});
	/* entity_new(c_name_new(level), c_level_new(level, 1)); */

	entity_new({
		c_name_new("ambient");
		c_light_new(-1.0f, vec4(0.5f, 0.5f, 0.5f, 0.5f));
	});
	c_level_set_active(c_level(&lvl), 1);
	entity_add_component(SYS, c_physics_new());

	/* ico = mesh_new(); */
	/* mesh_lock(ico); */
	/* mesh_ico(ico, 0.1f); */
	/* mesh_select(ico, SEL_EDITING, MESH_FACE, -1); */
	/* mesh_subdivide(ico, 2); */
	/* mesh_spherize(ico, 1.0f); */
	/* mesh_unlock(ico); */

	/* c_charlook_t *fc = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0); */
	mat_t *mat = mat_new("tt", "decal");
	entity_t decal = entity_new(c_decal_new(mat, 1, 1));
	c_spatial_rotate_Y(c_spatial(&decal), M_PI / 2.f);
	c_spatial_set_pos(c_spatial(&decal), vec3(1.52, 6.2, 0.6));

	mat1t(mat, ref("albedo.texture"), sauces("damage.png"));
	c_spatial_set_scale(c_spatial(&decal), vec3(1024.0f / 682.0f, 1.0, 1.0));

	/* mat1t(mat, ref("albedo.texture"), sauces("grunge.png")); */
	/* c_spatial_set_scale(c_spatial(&decal), vec3(1024.0f / 311.0f, 1.0, 1.0)); */

	/* mat4f(mat, ref("albedo.color"), vec4(0.0f)); */
	mat1f(mat, ref("albedo.blend"), 1.0f);
	/* mat4f(mat, ref("albedo.color"), vec4(1.0, 0.0, 0.0, 1.0)); */
	mat1f(mat, ref("metalness.value"), 0.f);
	mat1f(mat, ref("roughness.value"), 1.0f);


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
