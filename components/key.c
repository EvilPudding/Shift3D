#include "key.h"
#include "bridge.h"
#include "side.h"
#include "rigid_body.h"
#include "../candle/components/spatial.h"
#include "../candle/components/model.h"
#include "../candle/candle.h"
#include "../openal.candle/speaker.h"

static float c_rigid_body_key_collider(c_rigid_body_t *self, vec3_t pos)
{
	c_key_t *key = c_key(self);
	if(key->active != 1) return -1;
	entity_t ent = c_entity(self);

	const vec3_t kpos = c_spatial(self)->pos;
	c_side_t *side = c_side(self);

	c_model_t *model = c_model(self);
	if(model->visible && vec3_len(vec3_sub(pos, kpos)) < 0.6)
	{
		model->visible = 0;
		/* entity_signal(ent, spatial_changed, &ent); */

		ct_t *bridges = ecm_get(ct_bridge);

		khiter_t k;
		for(k = kh_begin(bridges->cs); k != kh_end(bridges->cs); ++k)
		{
			if(!kh_exist(bridges->cs, k)) continue;
			c_bridge_t *b = (c_bridge_t*)kh_value(bridges->cs, k);
			c_side_t *s = c_side(b);
			if(b->key == key->key && s->level == side->level)
			{
				b->rotate_to = key->rot;
				c_speaker_t *speaker = c_speaker(b);
				c_speaker_play(speaker, sauces("soundo.wav"), 0);
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
	return CONTINUE;
}

c_key_t *c_key_new(int rotX, int rotY, int rotZ, int key)
{
	c_key_t *self = component_new(ct_key);

    self->rot.x = ((float)rotX) * (M_PI / 180);
    self->rot.y = ((float)rotY) * (M_PI / 180);
    self->rot.z = ((float)rotZ) * (M_PI / 180);
	self->key = key;

	return self;
}

void ct_key(ct_t *self)
{
	ct_init(self, "key", sizeof(c_key_t));
	ct_add_dependency(self, ct_spatial);
	ct_add_listener(self, ENTITY, 0, ref("entity_created"), c_key_created);
}
