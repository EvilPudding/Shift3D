#include "bridge.h"
#include "rigid_body.h"
#include "../candle/components/spatial.h"
#include "../candle/components/model.h"
#include "../candle/candle.h"
#include "../openal.candle/speaker.h"

static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos);

c_bridge_t *c_bridge_new()
{
	c_bridge_t *self = component_new(ct_bridge);

	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_bridge_collider));

	entity_add_component(c_entity(self), c_speaker_new());
	return self;
}

static mat_t *g_bridge_mat;
void c_bridge_ready(c_bridge_t *self)
{
	mesh_t *mesh = mesh_new();
	mesh_cuboid(mesh, 0.5,
			vec3(self->min.x - 0.005f, self->min.y - 0.005f, self->min.z - 0.005f),
			vec3(self->max.x + 0.005f, self->max.y + 0.005f, self->max.z + 0.005f));

	if(!g_bridge_mat)
	{
		g_bridge_mat = mat_new("bridge", "transparent");
		mat1f(g_bridge_mat, ref("metalness.value"), 1.0f);
		mat1f(g_bridge_mat, ref("metalness.blend"), 0.0f);
		mat1t(g_bridge_mat, ref("roughness.texture"), sauces("rough.png"));
		mat1f(g_bridge_mat, ref("roughness.blend"), 0.8);
		/* mat1f(g_bridge_mat, ref("roughness.scale"), 0.2); */
		/* mat1f(g_bridge_mat, ref("normal.scale"), 2.0); */
		mat1t(g_bridge_mat, ref("normal.texture"), sauces("stone3_normal.png"));
		mat1f(g_bridge_mat, ref("normal.blend"), 0.3f);
		mat4f(g_bridge_mat, ref("absorb.color"), vec4(0.20f, 0.13f, 0.03f, 1.0));
	}

	entity_add_component(c_entity(self),
			c_model_new(mesh, g_bridge_mat, true, true));
}

void c_bridge_set_active(c_bridge_t *self, int active)
{
	/* if(active) */
	/* { */
		/* c_spatial_set_model(c_spatial(self), self->original_model); */
	/* } */
	self->active = active;
}

static int c_bridge_spatial_changed(c_bridge_t *self)
{
	c_spatial_t *spatial = c_spatial(self);
	self->inverse_model = mat4_invert(spatial->model_matrix);

	return CONTINUE;
}

static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_bridge_t *b = c_bridge(self);
	if(b->active != 1) return -1;
	/* c_spatial_t *b = c_spatial(c_entity(self)); */

	pos = vec4_xyz(mat4_mul_vec4(b->inverse_model, vec4(_vec3(pos), 1.0f)));

	/* float inc = 0;//-0.01; */
	int val = pos.x > b->min.x && pos.x < b->max.x
		&& pos.y > b->min.y && pos.y < b->max.y
		&& pos.z > b->min.z && pos.z < b->max.z;
	return val ? 4 : -1;
}

static int c_bridge_update(c_bridge_t *self, float *dt)
{
	if(!vec3_null(self->rotate_to))
	{
		c_spatial_t *s = c_spatial(self);
		c_spatial_lock(s);
		vec3_t inc;
		if(fabs(self->rotate_to.x) < 0.01 &&
				fabs(self->rotate_to.y) < 0.01 &&
				fabs(self->rotate_to.z) < 0.01)
		{
			inc = self->rotate_to;
		}
		else
		{
			inc = vec3_scale(self->rotate_to, (*dt) * 7.0f);
		}
		c_spatial_rotate_X(s, inc.x);
		c_spatial_rotate_Y(s, inc.y);
		c_spatial_rotate_Z(s, inc.z);
		c_spatial_unlock(s);

		self->rotate_to = vec3_sub(self->rotate_to, inc);
	}
	return CONTINUE;
}

void ct_bridge(ct_t *self)
{
	ct_init(self, "bridge", sizeof(c_bridge_t));
	ct_add_dependency(self, ct_node);

	ct_add_listener(self, ENTITY, 0, ref("spatial_changed"), c_bridge_spatial_changed);

	ct_add_listener(self, WORLD, 0, ref("world_update"), c_bridge_update);
}

