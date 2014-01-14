void fragment_func(out vec4 fragColor, out float fragDepth);

void main(void)
{
	fragment_func(FragData0, gl_FragDepth);
}

