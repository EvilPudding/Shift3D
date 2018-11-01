#include "common.glsl"
#line 4

BUFFER {
	sampler2D depth;
	sampler2D albedo;
	sampler2D nmr;
} gbuffer;

layout (location = 0) out vec4 AlbedoColor;

void main()
{
	vec2 fc = pixel_pos();
	float depth = textureLod(gbuffer.depth, fc, 0).r;
	if(depth < gl_FragCoord.z) discard;
	gl_FragDepth = gl_FragCoord.z;
	AlbedoColor = vec4(1);
}  

// vim: set ft=c:
