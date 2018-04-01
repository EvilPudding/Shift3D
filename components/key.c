#include "key.h"
#include "bridge.h"
#include <components/spacial.h>
#include <components/model.h>
#include <components/rigid_body.h>
#include <stdlib.h>

static float c_rigid_body_key_collider(c_rigid_body_t *self, vec3_t pos)
{
	entity_t ent = c_entity(self);

	const vec3_t kpos = c_spacial(self)->pos;

	c_model_t *model = c_model(self);
	if(model->visible && vec3_len(vec3_sub(pos, kpos)) < 0.4)
	{
		model->visible = 0;
		/* entity_signal(ent, spacial_changed, &ent); */

		c_key_t *key = c_key(self);
		ct_t *bridges = ecm_get(ref("c_bridge"));

		int i, p;
		for(p = 0; p < bridges->pages_size; p++)
		for(i = 0; i < bridges->pages[p].components_size; i++)
		{
			c_bridge_t *b = (c_bridge_t*)ct_get_at(bridges, p, i);
			if(b->key == key->key)
			{
				b->rotate_to = key->rot;
			}
		}
		entity_destroy(ent);
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
	c_key_t *self = component_new("c_key");

    self->rot.x = ((float)rotX) * (M_PI / 180);
    self->rot.y = ((float)rotY) * (M_PI / 180);
    self->rot.z = ((float)rotZ) * (M_PI / 180);
	self->key = key;

	return self;
}

REG()
{
	ct_t *ct = ct_new("c_key", sizeof(c_key_t), NULL, 1, ref("c_spacial"));

	signal_init(sig("key_activated"), sizeof(key_activated_data));

	ct_listener(ct, ENTITY, sig("entity_created"), c_key_created);
}
