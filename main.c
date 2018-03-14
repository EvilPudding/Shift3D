#include <candle.h>

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

#include "templates.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	char open_map_name[256];
	entity_t camera, g, body, character;
	candle_new(10,
			c_side_register,		c_grid_register,	c_charlook_register,
			c_character_register,	c_key_register,		c_level_register,
			c_bridge_register,		c_door_register,	c_movable_register,
			c_side_follow_register);

	entity_add_component(candle->systems, (c_t*)c_renderer_new(0.66f, 1, 1, 0));

	register_custom_templates(candle);

	g = entity_new(c_name_new("gravity"), c_force_new(0.0, -21, 0.0, 1));

	body = entity_new(c_name_new("body"), c_node_new());

	camera = entity_new(
			c_name_new("camera"),
			c_camera_new(70, 0.1, 50.0),
			c_charlook_new(body, 1.9)
	);
	c_renderer_add_camera(c_renderer(&candle->systems), camera);

	entity_add_component(candle->systems, (c_t*)c_editmode_new());

	character = entity_new(
			c_name_new("character"),
			c_character_new(body, 1, g)
	);

	c_spacial_set_pos(c_spacial(&camera), vec3(0.0, 0.7, 0.0));
	c_node_add(c_node(&character), 1, body);
	c_node_add(c_node(&body), 1, camera);

	/* entity_t venus = entity_new( */
	/* 		c_name_new("venus"), */
	/* 		c_model_new(sauces_mesh("venus.obj"), sauces_mat("pack1/stone3"), 1) */
	/* ); */
	/* c_spacial_set_pos(c_spacial(&venus), vec3(2.5, 5.5, 2.5)); */


	sprintf(open_map_name, "resauces/maps/%s.xmap", argc > 1 ? argv[1] : "0");

	entity_add_component(candle->systems, c_side_new(0));

	entity_add_component(candle->systems, c_level_new(candle, open_map_name));

	entity_t ambient = entity_new(c_ambient_new(64));
	c_spacial_set_pos(c_spacial(&ambient), vec3(6.5, 6, 6.5));

	/* entity_t decal = entity_new(c_decal_new(sauces_mat("pack1/piramids"))); */
	/* c_spacial_set_pos(c_spacial(&decal), vec3(10, 6, 5)); */

	/* entity_t decal = entity_new(c_sprite_new(sauces_mat("bridge"), 0)); */
	/* c_spacial_set_pos(c_spacial(&decal), vec3(10, 6, 5)); */

	//c_window_toggle_fullscreen(c_window(&candle->systems));

	candle_wait(candle);

	printf("Exiting.\n");

	return 0;
}
//*/
