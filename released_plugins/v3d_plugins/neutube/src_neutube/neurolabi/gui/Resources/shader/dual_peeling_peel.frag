//--------------------------------------------------------------------------------------
// Order Independent Transparency with Dual Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

//#extension ARB_draw_buffers : require

#ifdef USE_RECT_TEX
#if GLSL_VERSION >= 140
#define texture2DRect texture
#else
#extension GL_ARB_texture_rectangle : enable
#endif
uniform sampler2DRect DepthBlenderTex;
uniform sampler2DRect FrontBlenderTex;
#else
uniform sampler2D DepthBlenderTex;
uniform sampler2D FrontBlenderTex;
uniform vec2 screen_dim_RCP;
#endif

#define MAX_DEPTH 1.0

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void fragment_func(out vec4 fragColor, out float fragDepth);

void main(void)
{
	float fragDepth;
	vec4 color;
	fragment_func(color, fragDepth);

	#ifdef USE_RECT_TEX
	vec2 depthBlender = texture2DRect(DepthBlenderTex, gl_FragCoord.xy).xy;
	vec4 forwardTemp = texture2DRect(FrontBlenderTex, gl_FragCoord.xy);
	#else
	vec2 depthBlender = texture(DepthBlenderTex, gl_FragCoord.xy * screen_dim_RCP).xy;
	vec4 forwardTemp = texture(FrontBlenderTex, gl_FragCoord.xy * screen_dim_RCP);
	#endif

	// Depths and 1.0-alphaMult always increase
	// so we can use pass-through by default with MAX blending
	//FragData0.xy = depthBlender;

	// Front colors always increase (DST += SRC*ALPHA_MULT)
	// so we can use pass-through by default with MAX blending
	FragData1 = forwardTemp;

	// Because over blending makes color increase or decrease,
	// we cannot pass-through by default.
	// Each pass, only one fragment writes a color greater than 0
	FragData2 = vec4(0.0);

	float nearestDepth = -depthBlender.x;
	float farthestDepth = depthBlender.y;

	if (fragDepth < nearestDepth || fragDepth > farthestDepth) {
		// Skip this depth in the peeling algorithm
		FragData0.xy = vec2(-MAX_DEPTH);
		return;
	}

	if (fragDepth > nearestDepth && fragDepth < farthestDepth) {
		// This fragment needs to be peeled again
		FragData0.xy = vec2(-fragDepth, fragDepth);
		return;
	}

	// If we made it here, this fragment is on the peeled layer from last pass
	// therefore, we need to shade it, and make sure it is not peeled any farther
	FragData0.xy = vec2(-MAX_DEPTH);

	if (fragDepth == nearestDepth) {
		//FragData1.a = forwardTemp.a + (1.0 - forwardTemp.a) * color.a;
		//FragData1.rgb = (forwardTemp.rgb * forwardTemp.a + color.rgb * color.a * (1.0 - forwardTemp.a)) / FragData1.a;
		// use premultiplied alpha
		FragData1 = forwardTemp + (1.0 - forwardTemp.a) * color;
	} else {
		FragData2 += color;
	}
}
