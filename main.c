#include <candle.h>

#include "components/bridge.h"
#include "components/character.h"
#include "components/charlook.h"
#include "components/door.h"
#include "components/grid.h"
#include "components/key.h"
#include "components/level.h"
#include "components/moving.h"
#include "components/side.h"
#include "components/side_follow.h"

#include "templates.h"

#include <stdio.h>

void load_resources(candle_t *engine)
{
	/* TODO: make dir loading dynamic built in candle */
	/* loader_wait(engine->loader); */

	candle_material_get(engine, "blocks");
	candle_material_get(engine, "inverted");
	/* candle_material_get(engine, "movable")->diffuse.texture_blend =
	 * 0.5; */

	candle_material_get(engine, "key");
	candle_material_get(engine, "door");
	material_t *bridge_mat = candle_material_get(engine, "bridge");
	bridge_mat->diffuse.color = vec4(0.2f, 0.2f, 0.6f, 1.0f);
	bridge_mat->diffuse.texture_blend = 0.5;

	candle_mesh_get(engine, "key.obj");
	candle_mesh_get(engine, "door.obj");
	candle_mesh_reg(engine, "movable", mesh_cube(0.5f, 2.0f, 1));

	/* loader_wait(state->engine->loader); */
}

int main(int argc, char **argv)
{
	char open_map_name[256];
	entity_t camera, g, character, common;
	candle_new(10,
			c_side_register,		c_grid_register,	c_charlook_register,
			c_character_register,	c_key_register,		c_level_register,
			c_bridge_register,		c_door_register,	c_moving_register,
			c_side_follow_register);

	entity_add_component(candle->systems, (c_t*)c_renderer_new(1.0f, 1, 1, 1.0f, 0));
	entity_add_component(candle->systems, (c_t*)c_editmode_new());

	common = candle->ecm->common;

	register_custom_templates(candle);

	candle_get_materials_at(candle, "");
	load_resources(candle);

	g = entity_new(candle->ecm, 2, c_name_new("gravity"),
			c_force_new(0.0, -55, 0.0, 1));

	c_renderer(&candle->systems)->camera = camera = entity_new(candle->ecm, 4,
			c_name_new("camera"),
			c_camera_new(70, 0.1, 50.0),
			c_charlook_new(g, 1.9),
			c_node_new()
	);

	character = entity_new(candle->ecm, 2,
			c_name_new("character"),
			c_character_new(camera, 1, g)
	);

	/* c_charlook_set_controls(c_charlook(camera), character, camera); */
	c_charlook_set_controls(c_charlook(&camera), camera, camera);

	c_spacial_set_pos(c_spacial(&camera), vec3(0.0, 0.7, 0.0));
	c_node_add(c_node(&character), 1, camera);


	/* TORUS */
	/* entity_t torus = entity_new(candle->ecm, 2, */
			/* c_name_new("torus"), */
			/* c_model_new(mesh_torus(0.5, 0.2, 32, 16), */
				/* candle_material_get(candle, "stone5"), 1) */
	/* ); */
	/* c_spacial_set_pos(c_spacial(torus), vec3(11.0, 6.1, 7.0)); */
	/* c_spacial_set_rot(c_spacial(torus), 1, 0, 0, M_PI / 5.5); */


	sprintf(open_map_name, "resauces/maps/%s.xmap", argc > 1 ? argv[1] : "0");

	entity_add_component(common, (c_t*)c_side_new(0));

	entity_add_component(common, (c_t*)c_level_new(candle, open_map_name));

	entity_t ambient = entity_new(candle->ecm, 1, c_ambient_new(64));
	c_spacial_set_pos(c_spacial(&ambient), vec3(6.5, 6, 6.5));

	entity_signal(candle->ecm->none, window_resize,
			&(window_resize_data){window_width, window_height});

	candle_init(candle);


	printf("Exiting.\n");

	return 0;
}
//*/
