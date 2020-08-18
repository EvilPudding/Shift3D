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
