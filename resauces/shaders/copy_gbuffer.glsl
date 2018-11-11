
#include "common.glsl"
#line 4

layout (location = 0) out vec4 AlbedoColor;
layout (location = 1) out vec4 NMR; // normal_roughness_metalness

BUFFER {
	sampler2D depth;
	sampler2D albedo;
	sampler2D nmr;
} buf;


void main()
{
	vec4 alb = textureLod(buf.albedo, pixel_pos(), 0.0);
	if(alb.a == 0.0) discard;

	AlbedoColor = alb;
	NMR = textureLod(buf.nmr, pixel_pos(), 0.0);
	gl_FragDepth = textureLod(buf.depth, pixel_pos(), 0.0).r;
}

// vim: set ft=c:
