#include "common.glsl"
#line 4

layout (location = 0) out vec4 FragColor;

BUFFER {
	sampler2D color;
} traced;

BUFFER {
	sampler2D color;
} spheremap;

BUFFER {
	sampler2D color;
} spheremap2;

#define M_PI 3.1415926535897932384626433832795

void main()
{
	ivec2 tc = ivec2(int(gl_FragCoord.x), int(gl_FragCoord.y));
	vec4 pos = texelFetch(traced.color, tc, 0) * 2.0 - 1.0;
	float w = pos.w;
	vec3 wpos = normalize(pos.xyz);

	/* vec2 uv; */
	/* uv.x = atan(wpos.x, wpos.z) / (2*M_PI) + 0.5; */
	/* uv.y = wpos.y * 0.5 + 0.5; */

	highp vec2 uv;
	uv.x = (atan(wpos.z, wpos.x) / (2.0*M_PI) + 0.5);
	uv.y = 0.5 - asin(wpos.y) / M_PI;
	if(pos.a > 0)
	{
		FragColor = vec4(textureLod(spheremap.color, uv, 0).rgb, 1.0);
	}
	else
	{
		FragColor = vec4(textureLod(spheremap2.color, uv, 0).rgb, 1.0);
	}
}  

// vim: set ft=c:
