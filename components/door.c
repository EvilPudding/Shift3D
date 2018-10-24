#include "state.h"
#include "character.h"
#include "charlook.h"
#include "door.h"
#include "mirror.h"
#include "rigid_body.h"
#include <components/node.h>
#include <components/model.h>
#include <components/name.h>
#include <components/camera.h>
#include <components/spacial.h>
#include <stdlib.h>
#include <string.h>

static float c_rigid_body_door_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_spacial_t *door = c_spacial(self);
	c_spacial_t *character = c_spacial(ct_get_nth(ecm_get(ref("character")), 0));
	c_character_t *ch = c_character(character);

	pos = mat4_mul_vec4(mat4_invert(door->model_matrix), vec4(_vec3(pos), 1.0f)).xyz;

	if(fabs(pos.x) < 0.05 && fabs(pos.y - 0.5) < 0.5 && fabs(pos.z) < 0.3f)
	/* if(vec3_len(vec3_sub(pos, door->pos)) < 0.3) */
	{
		c_character_teleport(ch, c_entity(self), c_state(&SYS)->spawn);
		/* return 0; */
	}
	return -1;
}
renderer_t *shift_renderer(renderer_t *original);
c_door_t *c_door_new(const char *next)
{
	c_door_t *self = component_new("door");

	strcpy(self->next, next);

	c_charlook_t *cl = (c_charlook_t*)ct_get_nth(ecm_get(ref("charlook")), 0);
	c_camera_t *cam = c_camera(cl);

	self->mirror = entity_new(
			c_name_new("camera2"),
			c_camera_new(70, 0.1, 100.0, 0, 1, 1,
				shift_renderer(cam->renderer)),
			c_mirror_new(c_entity(cl), c_entity(self), c_state(&SYS)->spawn)
	);

	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_door_collider));

	return self;
}

int c_door_update(c_door_t *self, float *dt)
{
	c_spacial_t *sc = c_spacial(self);
	c_spacial_rotate_Y(sc, *dt * 0.1);
	return CONTINUE;
}

REG()
{
	ct_t *ct = ct_new("door", sizeof(c_door_t), NULL, NULL, 1, ref("node"));
	ct_listener(ct, WORLD, sig("world_update"), c_door_update);
}

