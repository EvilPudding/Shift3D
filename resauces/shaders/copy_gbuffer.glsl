
#include "common.glsl"
#line 4

layout (location = 0) out vec4 AlbedoColor;
layout (location = 1) out vec2 NN; // normal_roughness_metalness
layout (location = 2) out vec2 MR; // normal_roughness_metalness
layout (location = 3) out vec3 Emissive; // normal_roughness_metalness

BUFFER {
	sampler2D depth;
	sampler2D albedo;
	sampler2D nn;
	sampler2D mr;
	sampler2D emissive;
} buf;


void main()
{
	vec4 alb = textureLod(buf.albedo, pixel_pos(), 0.0);
	if(alb.a == 0.0) discard;

	AlbedoColor = alb;
	NN = textureLod(buf.nn, pixel_pos(), 0.0).rg;
	MR = textureLod(buf.mr, pixel_pos(), 0.0).rg;
	Emissive = textureLod(buf.emissive, pixel_pos(), 0.0).rgb;
	gl_FragDepth = textureLod(buf.depth, pixel_pos(), 0.0).r;
}

// vim: set ft=c:
