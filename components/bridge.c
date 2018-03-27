#include "bridge.h"
#include <components/rigid_body.h>
#include <components/spacial.h>
#include <components/model.h>
#include <stdlib.h>
#include <candle.h>


static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos);

void c_bridge_init(c_bridge_t *self)
{
	self->rotate_to = vec3(0.0);
}

c_bridge_t *c_bridge_new()
{
	c_bridge_t *self = component_new(ct_bridge);

	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_bridge_collider));

	return self;
}

void c_bridge_ready(c_bridge_t *self)
{

	mesh_t *mesh = mesh_cuboid(0.5,
			vec3(self->min.x - 0.005f, self->min.y - 0.005f, self->min.z - 0.005f),
			vec3(self->max.x + 0.005f, self->max.y + 0.005f, self->max.z + 0.005f));

	sauces_mat("bridge")->transparency.color = vec4(0.3, 0.3, 0.3, 1.0);

	entity_add_component(c_entity(self),
			c_model_new(mesh, sauces_mat("bridge"), 1));
}

static int c_bridge_spacial_changed(c_bridge_t *self)
{
	c_spacial_t *spacial = c_spacial(self);
	self->inverse_model = mat4_invert(spacial->model_matrix);

	return 1;
}

static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_bridge_t *b = c_bridge(self);
	/* c_spacial_t *b = c_spacial(c_entity(self)); */

	pos = mat4_mul_vec4(b->inverse_model, vec4(_vec3(pos), 1.0f)).xyz;

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
		c_spacial_t *s = c_spacial(self);
		c_spacial_lock(s);
		vec3_t inc;
		if(fabs(self->rotate_to.x) < 0.01 &&
				fabs(self->rotate_to.y) < 0.01 &&
				fabs(self->rotate_to.z) < 0.01)
		{
			inc = self->rotate_to;
		}
		else
		{
			inc = vec3_scale(self->rotate_to, (*dt) * 3.0f);
		}
		c_spacial_rotate_X(s, inc.x);
		c_spacial_rotate_Y(s, inc.y);
		c_spacial_rotate_Z(s, inc.z);
		c_spacial_unlock(s);

		self->rotate_to = vec3_sub(self->rotate_to, inc);
	}
	return 1;
}

DEC_CT(ct_bridge)
{
	ct_t *ct = ct_new("c_bridge", &ct_bridge,
			sizeof(c_bridge_t), (init_cb)c_bridge_init, 1, ct_spacial);

	/* ct_listener(ct, WORLD, collider_callback, c_bridge_collider); */

	ct_listener(ct, ENTITY, spacial_changed, c_bridge_spacial_changed);

	ct_listener(ct, WORLD, world_update, c_bridge_update);
}


/* void updateBridges(state_t *self) */
/* { */
/* 	int i; */
/* 	float x1, y1, z1, x2, y2, z2; */
/* 	entity_t lvl = &self->levels[0]; */
/* 	for(i=0;i<lvl->bridgeNum;i++) */
/* 	{ */
/* 		BRIDGE *bridge = &lvl->bridges[i]; */

/* 		if(abs(bridge->angFinx - bridge->angRotx) > 0.01 || */
/* 				abs(bridge->angFiny - bridge->angRoty) > 0.01 || */
/* 				abs(bridge->angFinz - bridge->angRotz) > 0.01) */
/* 		{ */
/* 			bridge->x1=bridge->ix1-0.5; */
/* 			bridge->y1=bridge->iy1-0.5; */
/* 			bridge->z1=bridge->iz1-0.5; */
/* 			bridge->x2=bridge->ix2+0.5; */
/* 			bridge->y2=bridge->iy2+0.5; */
/* 			bridge->z2=bridge->iz2+0.5; */

/* 			bridge->angRotx+=(bridge->angFinx-bridge->angRotx)/12; */
/* 			bridge->angRoty+=(bridge->angFiny-bridge->angRoty)/12; */
/* 			bridge->angRotz+=(bridge->angFinz-bridge->angRotz)/12; */

/* 			translatePoint(&(bridge->x1),&(bridge->y1),&(bridge->z1),-bridge->cx,-bridge->cy,-bridge->cz); */
/* 			translatePoint(&(bridge->x2),&(bridge->y2),&(bridge->z2),-bridge->cx,-bridge->cy,-bridge->cz); */
/* 			rotatePoint(&(bridge->x1),&(bridge->y1),&(bridge->z1),bridge->angFinx,bridge->angFiny,bridge->angFinz); */
/* 			rotatePoint(&(bridge->x2),&(bridge->y2),&(bridge->z2),bridge->angFinx,bridge->angFiny,bridge->angFinz); */
/* 			translatePoint(&(bridge->x1),&(bridge->y1),&(bridge->z1),bridge->cx,bridge->cy,bridge->cz); */
/* 			translatePoint(&(bridge->x2),&(bridge->y2),&(bridge->z2),bridge->cx,bridge->cy,bridge->cz); */

/* 			x1=bridge->x1; y1=bridge->y1; z1=bridge->z1; */
/* 			x2=bridge->x2; y2=bridge->y2; z2=bridge->z2; */


/* 			if(x1<x2) */
/* 			{ */
/* 				bridge->x1=x1;bridge->x2=x2; */
/* 			} */
/* 			else */
/* 			{ */
/* 				bridge->x1=x2;bridge->x2=x1; */
/* 			} */

/* 			if(y1<y2) */
/* 			{ */
/* 				bridge->y1=y1;bridge->y2=y2; */
/* 			} */
/* 			else */
/* 			{ */
/* 				bridge->y1=y2;bridge->y2=y1; */
/* 			} */

/* 			if(bridge->z1<bridge->z2) */
/* 			{ */
/* 				bridge->z1=z1;bridge->z2=z2; */
/* 			} */
/* 			else */
/* 			{ */
/* 				bridge->z1=z2;bridge->z2=z1; */
/* 			} */

/* 			entity_reset_model_matrix(bridge->entity); */
/* 			entity_translate(bridge->entity, bridge->cx, bridge->cy, bridge->cz); */
/* 			entity_set_rot(bridge->entity, 1, 0, 0, bridge->angRotx * (M_PI / 180.0f)); */
/* 			entity_set_rot(bridge->entity, 0, 1, 0, bridge->angRoty * (M_PI / 180.0f)); */
/* 			entity_set_rot(bridge->entity, 0, 0, 1, bridge->angRotz * (M_PI / 180.0f)); */
/* 			entity_translate(bridge->entity, -bridge->cx, -bridge->cy, -bridge->cz); */
/* 		} */
/* 	} */
/* } */

