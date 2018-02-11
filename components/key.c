#include "key.h"
#include "bridge.h"
#include <components/spacial.h>
#include <components/model.h>
#include <components/rigid_body.h>
#include <stdlib.h>

unsigned long ct_key;
unsigned long key_activated;

void c_key_init(c_key_t *self)
{
	self->super = component_new(ct_key);
}

static float c_rigid_body_key_collider(c_rigid_body_t *self, vec3_t pos)
{
	entity_t ent = c_entity(self);

	const vec3_t kpos = c_spacial(ent)->pos;

	c_model_t *model = c_model(ent);
	if(model->visible && vec3_len(vec3_sub(pos, kpos)) < 0.4)
	{
		model->visible = 0;
		entity_signal(ent, spacial_changed, &ent);

		c_key_t *key = c_key(ent);
		ct_t *bridges = ecm_get(c_ecm(self), ct_bridge);

		int i;
		for(i = 0; i < bridges->components_size; i++)
		{
			c_bridge_t *b = (c_bridge_t*)ct_get_at(bridges, i);
			if(b->key == key->key)
			{
				b->rotate_to = key->rot;
			}
		}
	}

	return -1;
}

static int c_key_created(c_key_t *self)
{
	entity_add_component(c_entity(self),
			(c_t*)c_rigid_body_new((collider_cb)c_rigid_body_key_collider));
	return 1;
}

c_key_t *c_key_new(int rotX, int rotY, int rotZ, int key)
{
	c_key_t *self = malloc(sizeof *self);
	c_key_init(self);

    self->rot.x = ((float)rotX) * (M_PI / 180);
    self->rot.y = ((float)rotY) * (M_PI / 180);
    self->rot.z = ((float)rotZ) * (M_PI / 180);
	self->key = key;

	return self;
}

void c_key_register(ecm_t *ecm)
{
	ct_t *ct = ecm_register(ecm, &ct_key, sizeof(c_key_t), (init_cb)c_key_init,
			1, ct_spacial);
	key_activated = ecm_register_signal(ecm, sizeof(key_activated_data));
	ct_register_listener(ct, SAME_ENTITY, entity_created,
			(signal_cb)c_key_created);
}
