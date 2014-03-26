uniform float alpha;

varying vec4 color;
uniform bool no_alpha = false;

void main(void)
{
  FragData0 = no_alpha ? color : vec4(color.rgb * color.a * alpha, color.a * alpha);
}

