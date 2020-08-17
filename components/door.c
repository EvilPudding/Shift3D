#include "level.h"
#include "character.h"
#include "charlook.h"
#include "door.h"
#include "mirror.h"
#include "side.h"
#include "rigid_body.h"
#include <components/node.h>
#include <components/model.h>
#include <components/name.h>
#include <components/camera.h>
#include <components/spatial.h>
#include <stdlib.h>
#include <string.h>

static int c_door_position_changed(c_door_t *self);

static float c_rigid_body_door_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_spatial_t *door = c_spatial(self);
	c_door_t *dc = c_door(self);
	if(dc->active != 1) return -1;
	c_character_t *ch = (c_character_t*)ct_get_nth(ecm_get(ref("character")), 0);
	int side = c_side(dc)->side & 1;
	if(side != (c_side(ch)->side & 1)) return -1;

	pos = vec4_xyz(mat4_mul_vec4(mat4_invert(door->model_matrix), vec4(_vec3(pos), 1.0f)));

	if(fabs(pos.x) < 0.1 && fabs(pos.y + (side ? 0.5 : -0.5)) < 0.5 && fabs(pos.z) < 0.3f)
	/* if(vec3_len(vec3_sub(pos, door->pos)) < 0.3) */
	{
		c_character_teleport(ch, c_entity(self),
				c_level(&dc->next_level)->spawn);
		/* return 0; */
	}
	return -1;
}
static mesh_t *g_portal_mesh;
c_door_t *c_door_new(const char *next)
{
	c_door_t *self = component_new("door");

	if(!g_portal_mesh)
	{
		g_portal_mesh = mesh_new();
		mesh_add_regular_quad(g_portal_mesh,
				vec3(0.0f, 0.0f, 0.3f), Z3, Z2, vec3(0.0f, 0.0f, -0.3f), Z3, Z2,
				vec3(0.0f, 0.9f, -0.3f), Z3, Z2, vec3(0.0f, 0.9f, 0.3f), Z3, Z2);
		g_portal_mesh->cull = 0;
	}

	drawable_init(&self->draw, 0);
	drawable_set_vs(&self->draw, model_vs());
	drawable_set_mesh(&self->draw, g_portal_mesh);

	strcpy(self->next, next);

	return self;
}

entity_t level_loaded(const char *next)
{
	int i;
	c_node_t *node = c_node(&SYS);
	for(i = 0; i < node->children_size; i++)
	{
		c_name_t *name = c_name(&node->children[i]);
		if(name && name->name[0])
		{
			if(!strcmp(next, name->name))
			{
				return node->children[i];
			}
		}
	}
	return entity_null;
}


void c_door_set_active(c_door_t *self, int active)
{
	if(active == 1 && self->active != 1)
	{
		
		self->next_level = level_loaded(self->next);
		if(!entity_exists(self->next_level))
		{
			self->next_level = entity_new({
				c_name_new(self->next);
				c_level_new(self->next, 2);
			});
		}

		drawable_add_group(&self->draw, ref("portal"));

		entity_add_component(c_entity(self),
				(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_door_collider));
	}
	else
	{
		drawable_remove_group(&self->draw, ref("portal"));
	}
	c_door_position_changed(self);

	self->active = active;
}

static int c_door_pre_draw(c_door_t *self)
{
	if(!self->modified) return CONTINUE;

	c_node_t *node = c_node(self);
	c_node_update_model(node);

	drawable_set_transform(&self->draw, node->model);
	self->modified = 0;
	return CONTINUE;
}

static int c_door_position_changed(c_door_t *self)
{
	self->modified = 1;
	return CONTINUE;
}

void c_door_destroy(c_door_t *self)
{
	drawable_set_mesh(&self->draw, NULL);
}

void ct_door(ct_t *self)
{
	ct_init(self, "door", sizeof(c_door_t));
	ct_set_destroy(self, (destroy_cb)c_door_destroy);
	ct_set_dependency(self, ct_node);
	ct_add_listener(ct, ENTITY, 0, ref("node_changed"), c_door_position_changed);
	ct_add_listener(ct, WORLD,  0, ref("world_pre_draw"), c_door_pre_draw);
}

