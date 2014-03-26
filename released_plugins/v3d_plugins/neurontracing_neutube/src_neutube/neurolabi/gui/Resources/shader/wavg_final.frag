//--------------------------------------------------------------------------------------
// Order Independent Transparency with Average Color
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#ifdef USE_RECT_TEX
#if GLSL_VERSION >= 140
#define texture2DRect texture
#else
#extension GL_ARB_texture_rectangle : enable
#endif
uniform sampler2DRect ColorTex0;
uniform sampler2DRect ColorTex1;
#else
uniform sampler2D ColorTex0;
uniform sampler2D ColorTex1;
uniform vec2 screen_dim_RCP;
#endif

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main(void)
{
	#ifdef USE_RECT_TEX
	vec4 SumColor = texture2DRect(ColorTex0, gl_FragCoord.xy);
	vec2 nandd = texture2DRect(ColorTex1, gl_FragCoord.xy).xy;
	#else
	vec4 SumColor = texture(ColorTex0, gl_FragCoord.xy * screen_dim_RCP);
	vec2 nandd = texture(ColorTex1, gl_FragCoord.xy * screen_dim_RCP).xy;
	#endif

	float n = nandd.x;

	if (n == 0.0) {
		discard;
	}

	vec3 AvgColor = SumColor.rgb / SumColor.a;
	float AvgAlpha = SumColor.a / n;

	float T = pow(1.0-AvgAlpha, n);
	FragData0.a = 1 - T;
	FragData0.rgb = AvgColor * FragData0.a;
	gl_FragDepth = nandd.y / n;
}
