uniform float alpha;

varying vec4 color;
uniform bool no_alpha = false;

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  fragDepth = gl_FragCoord.z;
  fragColor = no_alpha ? color : vec4(color.rgb * color.a * alpha, color.a * alpha);
}

