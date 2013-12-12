//--------------------------------------------------------------------------------------
// Order Independent Transparency with Dual Depth Peeling
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
uniform sampler2DRect TempTex;
#else
uniform sampler2D TempTex;
uniform vec2 screen_dim_RCP;
#endif

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main(void)
{
	#ifdef USE_RECT_TEX
	FragData0 = texture2DRect(TempTex, gl_FragCoord.xy);
	#else
	FragData0 = texture(TempTex, gl_FragCoord.xy * screen_dim_RCP);
	#endif
	// for occlusion query
	if (FragData0.a == 0) discard;
}
