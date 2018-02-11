#include "bridge.h"
#include <components/rigid_body.h>
#include <components/spacial.h>
#include <stdlib.h>

unsigned long ct_bridge;

void c_bridge_init(c_bridge_t *self)
{
	self->super = component_new(ct_bridge);

    self->pivot = vec3(0.0);
    self->rotate_to = vec3(0.0);
}

c_bridge_t *c_bridge_new()
{
	c_bridge_t *self = malloc(sizeof *self);
	c_bridge_init(self);

	return self;
}

static float c_rigid_body_bridge_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_bridge_t *b = c_bridge(c_entity(self));
	/* c_spacial_t *b = c_spacial(c_entity(self)); */

	float inc = 0;//-0.01;
	int val = pos.x > b->x1 + inc && pos.x < b->x2 - inc
		&& pos.y > b->y1 + inc && pos.y < b->y2 - inc
		&& pos.z > b->z1 + inc && pos.z < b->z2 - inc;
	return val ? 4 : -1;
}

static int c_bridge_created(c_bridge_t *self)
{
	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_bridge_collider));
	return 1;
}

void c_bridge_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, &ct_bridge, sizeof(c_bridge_t),
			(init_cb)c_bridge_init, 1, ct_spacial);
	ct_register_listener(ct, SAME_ENTITY, entity_created,
			(signal_cb)c_bridge_created);
	/* ct_register_listener(ct, WORLD, collider_callback, */
			/* (signal_cb)c_bridge_collider); */
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

