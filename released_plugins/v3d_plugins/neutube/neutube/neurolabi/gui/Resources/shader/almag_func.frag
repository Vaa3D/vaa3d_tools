uniform sampler2D tex;

uniform float alpha;
#ifdef USE_SOFTEDGE
uniform float softedge_scale;
#endif
#if defined(SHOW_GLOW) || defined(SHOW_OUTLINE)
uniform vec4 outline_color;
#endif
#ifdef SHOW_SHADOW
uniform vec4 shadow_color;
#endif

varying vec2 texCoord0;
varying vec4 color;

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void fragment_func(out vec4 fragColor, out float fragDepth)
{
	vec4 baseColor = vec4(color.xyz, 1.0);
	float distanceFactor = texture(tex, texCoord0).a;

#ifdef USE_SOFTEDGE
	float width = fwidth(texCoord0.x) * softedge_scale;
	baseColor.a = smoothstep(0.5-width, 0.5+width, distanceFactor);
#else
	baseColor.a = baseColor.a >= 0.5 ? 1.0 : 0.0;
#endif

#ifdef SHOW_GLOW
	const float OUTER_GLOW_MIN = 0.2;
	const float OUTER_GLOW_MAX = 0.5;
	float glowFactor = smoothstep(OUTER_GLOW_MIN, OUTER_GLOW_MAX, distanceFactor);
	baseColor = mix(vec4(outline_color.xyz, glowFactor), baseColor, baseColor.a);
#endif

#ifdef SHOW_OUTLINE
	// Outline constants
	const float OUTLINE_MIN_0 = 0.3;
	float OUTLINE_MIN_1 = 0.31;

	const float OUTLINE_MAX_0 = 0.5;
	float OUTLINE_MAX_1 = 0.51;

	// Outline calculation
	if (distanceFactor > OUTLINE_MIN_0 && distanceFactor < OUTLINE_MAX_1)
	{
		float outlineAlpha;
		if (distanceFactor < OUTLINE_MIN_1)
			outlineAlpha = smoothstep(OUTLINE_MIN_0, OUTLINE_MIN_1, distanceFactor);
		else
			outlineAlpha = smoothstep(OUTLINE_MAX_1, OUTLINE_MAX_0, distanceFactor);

		baseColor = mix(baseColor, outline_color, outlineAlpha);
	}
#endif

#ifdef SHOW_SHADOW
	// Shadow / glow constants
	const vec2 GLOW_UV_OFFSET = vec2(-0.0015, -0.0015);

	// Shadow / glow calculation
	float glowDistance = texture(tex, texCoord0 + GLOW_UV_OFFSET).a;
	float glowFactor = smoothstep(0.3, 0.5, glowDistance);

	baseColor = mix(vec4(shadow_color.xyz, glowFactor), baseColor, baseColor.a);
#endif

	baseColor.a = baseColor.a * alpha;
	baseColor.rgb *= baseColor.a;

	fragColor = baseColor;
	fragDepth = gl_FragCoord.z;
}