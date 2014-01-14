uniform vec2 screen_dim_RCP;

uniform sampler2D color_texture;
uniform sampler2D depth_texture;
uniform float output_color_option = 0.0;
uniform bool discard_transparent = false;

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main()
{
  vec2 texCoords = gl_FragCoord.xy * screen_dim_RCP;
  vec4 fragColor = texture(color_texture, texCoords);

  if (discard_transparent && fragColor.a == 0.0)
    discard;

  if (output_color_option > 0.0)
    fragColor.rgb *= fragColor.a;
  else if (fragColor.a > 0.0 && output_color_option < 0.0)
    fragColor.rgb /= fragColor.a;

  FragData0 = fragColor;

  gl_FragDepth = texture(depth_texture, texCoords).z;
}
