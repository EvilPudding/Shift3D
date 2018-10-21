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

static mesh_t *ico;
void spawn_sprite(entity_t par)
{
	float x = ((float)(rand()%120)) / 10.0f;
	float y = ((float)(rand()%120)) / 24.0f + 6.0f;
	float z = ((float)(rand()%120)) / 10.0f;
	float s = ((float)(rand()%98)) / 100.0f + 0.02f;
	entity_t sprite = entity_new(c_model_new(ico, NULL, 0, 1));
	c_spacial_t *sc = c_spacial(&sprite);
	c_spacial_lock(sc);
	c_spacial_set_pos(sc, vec3(x, y, z));
	c_spacial_set_scale(sc, vec3(s));
	c_spacial_unlock(sc);

	c_node_add(c_node(&par), 1, sprite);

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
	/* entity_add_component(SYS, c_renderer_new(1.0f, 1, 1, 0)); */
	entity_add_component(SYS, (c_t*)c_editmode_new());
	/* c_editmode_activate(c_editmode(&SYS)); */

	c_sauces_index_dir(c_sauces(&SYS), "resauces");

	reg_custom_cmds();

	g = entity_new(c_name_new("gravity"), c_force_new(0.0, -21, 0.0, 1));

	body = entity_new(c_name_new("body"), c_node_new());

	camera = entity_new(
			c_name_new("camera"),
			c_camera_new(70, 0.1, 50.0, renderer_new(0.66f, 1, 1, 0)),
			c_charlook_new(body, 1.9)
	);
	/* c_window_set_renderer(c_window(&SYS), c_camera(camera)->renderer); */

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
		/* spawn_sprite(par); */
	/* } */

	//c_window_toggle_fullscreen(c_window(&candle->systems));

	candle_wait();

	printf("Exiting.\n");

	return 0;
}
//*/
