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

#include <components/node.h>
#include <components/force.h>
#include <components/name.h>
#include <components/camera.h>
#include <components/ambient.h>
#include <components/decal.h>
#include <components/sprite.h>
#include <components/light.h>
#include <systems/renderer.h>
#include <systems/editmode.h>

#include "cmds.h"

#include <stdio.h>

int main(int argc, char **argv)
{
	char open_map_name[256];
	entity_t camera, g, body, character;

	entity_add_component(SYS, (c_t*)c_renderer_new(0.66f, 1, 1, 0));

	reg_custom_cmds();

	g = entity_new(c_name_new("gravity"), c_force_new(0.0, -21, 0.0, 1));

	body = entity_new(c_name_new("body"), c_node_new());

	camera = entity_new(
			c_name_new("camera"),
			c_camera_new(70, 0.1, 50.0),
			c_charlook_new(body, 1.9)
	);
	c_renderer_add_camera(c_renderer(&SYS), camera);

	entity_add_component(SYS, (c_t*)c_editmode_new());

	character = entity_new(
			c_name_new("character"),
			c_character_new(body, 1, g)
	);

	c_spacial_set_pos(c_spacial(&camera), vec3(0.0, 0.7, 0.0));
	c_node_add(c_node(&character), 1, body);
	c_node_add(c_node(&body), 1, camera);

	sprintf(open_map_name, "resauces/maps/%s.xmap", argc > 1 ? argv[1] : "0");

	entity_add_component(SYS, c_side_new(0));

	entity_add_component(SYS, c_level_new(open_map_name));

	entity_new(c_name_new("ambient"), c_light_new(0.08f, -1.0f, vec4(1.0f), 0));

	entity_t decal = entity_new(c_decal_new(sauces_mat("pack1/piramids")));
	c_spacial_set_pos(c_spacial(&decal), vec3(10, 6, 10));
	c_spacial_rotate_Y(c_spacial(&decal), M_PI / 5);
	c_spacial_rotate_X(c_spacial(&decal), -M_PI / 2);

	entity_t sprite = entity_new(c_sprite_new(sauces_mat("bridge"), 0));
	c_spacial_set_pos(c_spacial(&sprite), vec3(10, 6, 5));

	//c_window_toggle_fullscreen(c_window(&candle->systems));

	candle_wait();

	printf("Exiting.\n");

	return 0;
}
//*/
