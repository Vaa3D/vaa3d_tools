// note: this shader requires that base radius is smaller than top radius

attribute vec4 attr_origin;    // base location + base radius
attribute vec4 attr_axis;      // axis (= top - base) + top radius
attribute float attr_flags;
attribute vec4 attr_colors;
attribute vec4 attr_colors2;

uniform float size_scale = 1.0;
uniform vec3 pos_scale = vec3(1.0, 1.0, 1.0);
uniform mat4 view_matrix;
uniform mat4 projection_view_matrix;
uniform mat4 projection_matrix_inverse;
uniform mat3 normal_matrix;

#if GLSL_VERSION >= 130 && defined(HAS_CLIP_PLANE)
uniform vec4 clip_planes[CLIP_PLANE_COUNT];
#endif

varying vec3 point;
varying vec3 axis;
varying vec3 base;
varying vec3 top;
varying vec3 U;
varying vec3 V;
varying vec4 combo1;
#define bradius combo1.x
#define tradius combo1.y
#define height combo1.z
#define inv_sqr_height combo1.w
//varying float bradius;
//varying float tradius;
//varying float height;
//varying float inv_sqr_height;
varying vec4 color1;
varying vec4 color2;

void main(void)
{
  bradius = size_scale * attr_origin.w;
  tradius = size_scale * attr_axis.w;

  //color1 = use_alpha ? vec4(attr_colors.xyz, alpha) : attr_colors;
  //color2 = use_alpha ? vec4(attr_colors2.xyz, alpha) : attr_colors2;
  color1 = attr_colors;
  color2 = attr_colors2;

  vec2 flags = mod(floor(vec2(attr_flags/16.0, attr_flags)), 16.0);
  // either 0 or 1, 0 -> left, 1 -> right
  float rightFlag = flags.x;
  // either 0 or 1, 0 -> down, 1 -> up
  float upFlag = flags.y;

  vec3 scaledAxis = attr_axis.xyz * pos_scale;
  vec3 scaledOrigin = attr_origin.xyz * pos_scale;

  height = length(scaledAxis);
  inv_sqr_height = height * height;
  inv_sqr_height = 1.0 / inv_sqr_height;

  // h is a normalized cylinder axis
  vec3 h = normalize(scaledAxis);

  // axis is the cylinder axis in modelview coordinates
  axis =  normalize(normal_matrix * h);

  // u, v, h is local system of coordinates
  vec3 u = cross(h, vec3(1.0, 0.0, 0.0));
  if (dot(u,u) < 0.001)
    u = cross(h, vec3(0.0, 1.0, 0.0));
  u = normalize(u);
  vec3 v = normalize(cross(u, h));

  // transform to modelview coordinates
  U = normalize(normal_matrix * u);
  V = normalize(normal_matrix * v);

  // compute bounding box vertex position

#if defined(FLAT_CAPS) || defined(NO_CAPS)
  vec4 p1 = vec4(scaledOrigin + bradius * (u+v), 1.0);
  vec4 p2 = vec4(scaledOrigin + bradius * (-u+v), 1.0);
  vec4 p3 = vec4(scaledOrigin + bradius * (-u-v), 1.0);
  vec4 p4 = vec4(scaledOrigin + bradius * (u-v), 1.0);
  vec4 p5 = vec4(scaledOrigin + scaledAxis.xyz + tradius * (u+v), 1.0);
  vec4 p6 = vec4(scaledOrigin + scaledAxis.xyz + tradius * (-u+v), 1.0);
  vec4 p7 = vec4(scaledOrigin + scaledAxis.xyz + tradius * (-u-v), 1.0);
  vec4 p8 = vec4(scaledOrigin + scaledAxis.xyz + tradius * (u-v), 1.0);
#else
  float adjustedTopRadius = tradius + tradius * (tradius - bradius) / height;

  vec4 p1 = vec4(scaledOrigin + bradius * (u+v-h), 1.0);
  vec4 p2 = vec4(scaledOrigin + bradius * (-u+v-h), 1.0);
  vec4 p3 = vec4(scaledOrigin + bradius * (-u-v-h), 1.0);
  vec4 p4 = vec4(scaledOrigin + bradius * (u-v-h), 1.0);
  vec4 p5 = vec4(scaledOrigin + scaledAxis.xyz + adjustedTopRadius * (u+v) + tradius * h, 1.0);
  vec4 p6 = vec4(scaledOrigin + scaledAxis.xyz + adjustedTopRadius * (-u+v) + tradius * h, 1.0);
  vec4 p7 = vec4(scaledOrigin + scaledAxis.xyz + adjustedTopRadius * (-u-v) + tradius * h, 1.0);
  vec4 p8 = vec4(scaledOrigin + scaledAxis.xyz + adjustedTopRadius * (u-v) + tradius * h, 1.0);
#endif

  p1 = projection_view_matrix * p1;
  p2 = projection_view_matrix * p2;
  p3 = projection_view_matrix * p3;
  p4 = projection_view_matrix * p4;
  p5 = projection_view_matrix * p5;
  p6 = projection_view_matrix * p6;
  p7 = projection_view_matrix * p7;
  p8 = projection_view_matrix * p8;

  p1.xyz = p1.xyz / p1.w;
  p2.xyz = p2.xyz / p2.w;
  p3.xyz = p3.xyz / p3.w;
  p4.xyz = p4.xyz / p4.w;
  p5.xyz = p5.xyz / p5.w;
  p6.xyz = p6.xyz / p6.w;
  p7.xyz = p7.xyz / p7.w;
  p8.xyz = p8.xyz / p8.w;

  vec4 pmin = p1;
  pmin = min(pmin, p2);
  pmin = min(pmin, p3);
  pmin = min(pmin, p4);
  pmin = min(pmin, p5);
  pmin = min(pmin, p6);
  pmin = min(pmin, p7);
  pmin = min(pmin, p8);

  vec4 pmax = p1;
  pmax = max(pmax, p2);
  pmax = max(pmax, p3);
  pmax = max(pmax, p4);
  pmax = max(pmax, p5);
  pmax = max(pmax, p6);
  pmax = max(pmax, p7);
  pmax = max(pmax, p8);

  float depth = pmin.z < -1.0 && pmax.z > -1.0 ? -0.999 : pmin.z;
  //float depth = min(pmin.z, pmax.z);
  if (pmin.x < -1.0 && pmax.x > 1.0 && pmin.y < -1.0 && pmax.y > 1.0) {
    depth = -2.0;
  }

  vec4 vertex = vec4(mix(pmin.x, pmax.x, rightFlag), mix(pmin.y, pmax.y, upFlag),
                     depth, 1.0);

  vec4 base4 = view_matrix * vec4(scaledOrigin, 1.0);
  base = base4.xyz;

  vec4 top4 = view_matrix * vec4(scaledOrigin + scaledAxis, 1.0);
  top = top4.xyz;

  vec4 tvertex = projection_matrix_inverse * vertex;
  point = tvertex.xyz / tvertex.w;

  gl_Position = vertex;
#if defined(HAS_CLIP_PLANE)
#if GLSL_VERSION >= 130
  for (int i=0; i<CLIP_PLANE_COUNT; ++i)
    gl_ClipDistance[i] = dot(clip_planes[i], vec4(scaledOrigin + scaledAxis * .5, 1.0));
#else
  gl_ClipVertex = vec4(scaledOrigin + scaledAxis * 0.5, 1.0);
#endif   // version 130 or up
#endif  // has clipplane
}

