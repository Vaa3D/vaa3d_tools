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
uniform sampler2DRect DepthTex;
uniform sampler2DRect FrontBlenderTex;
uniform sampler2DRect BackBlenderTex;
#else
uniform sampler2D DepthTex;
uniform sampler2D FrontBlenderTex;
uniform sampler2D BackBlenderTex;
uniform vec2 screen_dim_RCP;
#endif

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main(void)
{
	#ifdef USE_RECT_TEX
	vec4 frontColor = texture2DRect(FrontBlenderTex, gl_FragCoord.xy);
	vec4 backColor = texture2DRect(BackBlenderTex, gl_FragCoord.xy);
	gl_FragDepth = -texture2DRect(DepthTex, gl_FragCoord.xy).x;
	#else
	vec4 frontColor = texture(FrontBlenderTex, gl_FragCoord.xy * screen_dim_RCP);
	vec4 backColor = texture(BackBlenderTex, gl_FragCoord.xy * screen_dim_RCP);
	gl_FragDepth = -texture(DepthTex, gl_FragCoord.xy * screen_dim_RCP).x;
	#endif

	// front + back
	//FragData0.a = frontColor.a + (1.0 - frontColor.a) * backColor.a;
	//FragData0.rgb = (frontColor.rgb * frontColor.a + backColor.rgb * backColor.a * (1.0 - frontColor.a)) / FragData0.a;
	// use premultiplied alpha
	FragData0 = frontColor + (1.0 - frontColor.a) * backColor;


	// front blender
	//gl_FragColor.rgb = frontColor + vec3(alphaMultiplier);

	// back blender
	//gl_FragColor.rgb = backColor;
}
