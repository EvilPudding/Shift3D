#include "common.glsl"

layout (location = 0) out vec4 AlbedoColor;
layout (location = 1) out vec4 NMR; // normal_roughness_metalness


BUFFER {
	sampler2D depth;
} portal;

void main()
{
	float depth = textureLod(portal.depth, pixel_pos(), 0.0).r;
	if(depth == 0.0) discard;
	if(gl_FragCoord.z < depth) discard;

	vec4 dif  = resolveProperty(mat(albedo), texcoord, false);
	if(dif.a < 0.7) discard;

	dif.rgb += poly_color;
	/* dif.rgb = TM[2] / 2.0 + 0.5; */
	AlbedoColor = dif;

	NMR.rg = encode_normal(get_normal());

	NMR.b = resolveProperty(mat(metalness), texcoord, false).r;
	NMR.a = resolveProperty(mat(roughness), texcoord, false).r;

	/* AlbedoColor = scene.test_color; */
}

// vim: set ft=c:
