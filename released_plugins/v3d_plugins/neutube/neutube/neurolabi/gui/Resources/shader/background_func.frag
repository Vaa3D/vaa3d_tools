
uniform vec2 screen_dim_RCP;

uniform vec4 color1;
#if !defined(UNIFORM)
uniform vec4 color2;
#endif

void fragment_func(out vec4 fragColor, out float fragDepth)
{
#ifdef UNIFORM
	fragColor = color1;
#endif

#ifdef GRADIENT_LEFT_TO_RIGHT
	fragColor = mix(color1, color2, gl_FragCoord.x * screen_dim_RCP.x);
#endif

#ifdef GRADIENT_RIGHT_TO_LEFT
	fragColor = mix(color2, color1, gl_FragCoord.x * screen_dim_RCP.x);
#endif

#ifdef GRADIENT_TOP_TO_BOTTOM
	fragColor = mix(color2, color1, gl_FragCoord.y * screen_dim_RCP.y);
#endif

#ifdef GRADIENT_BOTTOM_TO_TOP
	fragColor = mix(color1, color2, gl_FragCoord.y * screen_dim_RCP.y);
#endif

	fragDepth = gl_FragCoord.z;
}

