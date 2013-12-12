uniform vec2 screen_dim_RCP;

uniform sampler2D color_texture_0;
uniform sampler2D depth_texture_0;
uniform sampler2D color_texture_1;
uniform sampler2D depth_texture_1;

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main()
{
  vec2 texCoords = gl_FragCoord.xy * screen_dim_RCP;

  vec4 color0 = texture(color_texture_0, texCoords);
  float depth0 = texture(depth_texture_0, texCoords).z;
  vec4 color1 = texture(color_texture_1, texCoords);
  float depth1 = texture(depth_texture_1, texCoords).z;

  vec4 fragColor = vec4(0.0);
  float fragDepth = 1.0;

#if defined(DEPTH_TEST)
  if (depth0 < depth1) {
    fragColor = color0;
    fragDepth = depth0;
  } else {
    fragColor = color1;
    fragDepth = depth1;
  }
#elif defined(FIRST_ON_TOP)
  if (color0.a > 0.0) {
    fragColor = color0;
    fragDepth = depth0;
  } else if (color1.a > 0.0) {
    fragColor = color1;
    fragDepth = depth1;
  }
#elif defined(SECOND_ON_TOP)
  if (color1.a > 0.0) {
    fragColor = color1;
    fragDepth = depth1;
  } else if (color0.a > 0.0) {
    fragColor = color0;
    fragDepth = depth0;
  }
#elif defined(DEPTH_TEST_BLENDING)
  // from wikipedia
  if (depth1 < depth0) {
    // fragColor.a = color1.a + color0.a * (1.0 - color1.a);
    // if (fragColor.a > 0.0)
    //   fragColor.rgb = (color1.a * color1.rgb + color0.a * (1.0 - color1.a) * color0.rgb) / fragColor.a;
    // use premultiplied alpha
    fragColor = color1 + (1 - color1.a) * color0;
  } else {
    // fragColor.a = color0.a + color1.a * (1.0 - color0.a);
    // if (fragColor.a > 0.0)
    //   fragColor.rgb = (color0.a * color0.rgb + color1.a * (1.0 - color0.a) * color1.rgb) / fragColor.a;
    // use premultiplied alpha
    fragColor = color0 + (1 - color0.a) * color1;
  }
  fragDepth = min(depth0, depth1);
#elif defined(FIRST_ON_TOP_BLENDING)
  // fragColor.a = color0.a + color1.a * (1.0 - color0.a);
  // if (fragColor.a > 0.0) {
  //   fragColor.rgb = (color0.a * color0.rgb + color1.a * (1.0 - color0.a) * color1.rgb) / fragColor.a;
  //   fragDepth = color0.a > 0.0 ? depth0 : depth1;
  // }
  // use premultiplied alpha
  fragColor = color0 + (1 - color0.a) * color1;
  fragDepth = color0.a > 0.0 ? depth0 : depth1;
#elif defined(SECOND_ON_TOP_BLENDING)
  // fragColor.a = color1.a + color0.a * (1.0 - color1.a);
  // if (fragColor.a > 0.0) {
  //   fragColor.rgb = (color1.a * color1.rgb + color0.a * (1.0 - color1.a) * color0.rgb) / fragColor.a;
  //   fragDepth = color1.a > 0.0 ? depth1 : depth0;
  // }
  // use premultiplied alpha
  fragColor = color1 + (1 - color1.a) * color0;
  fragDepth = color1.a > 0.0 ? depth1 : depth0;
#endif

  FragData0 = fragColor;
  gl_FragDepth = fragDepth;
}
