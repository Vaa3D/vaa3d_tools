attribute vec4 attr_vertex_radius;
#ifdef DYNAMIC_MATERIAL_PROPERTY
attribute vec4 attr_specular_shininess;
#endif
attribute vec4 attr_color;
attribute float attr_flags;

uniform vec3 pos_scale = vec3(1.0, 1.0, 1.0);
uniform float size_scale = 1.0;
uniform float box_correction = 1.5;
uniform mat4 view_matrix;
uniform mat4 projection_view_matrix;
#if GLSL_VERSION >= 130 && defined(HAS_CLIP_PLANE)
uniform vec4 clip_planes[CLIP_PLANE_COUNT];
#endif

varying vec4 color;
varying vec3 sphere_center;
varying vec3 point;
varying float radius2;
#ifdef DYNAMIC_MATERIAL_PROPERTY
varying float va_material_shininess;
varying vec4 va_material_specular;
#endif

void main(void)
{
  float radius = attr_vertex_radius.w * size_scale;

  vec2 flags = mod(floor(vec2(attr_flags/16.0, attr_flags)), 16.0);
  // either -1 or 1, -1 -> left, 1 -> right
  float rightFlag = flags.x - 1.;
  // either -1 or 1, -1 -> down, 1 -> up
  float upFlag = flags.y - 1.;

  vec4 centerVertex = vec4(attr_vertex_radius.xyz * pos_scale, 1.0);

  color = attr_color;

#ifdef DYNAMIC_MATERIAL_PROPERTY
  va_material_specular = vec4(attr_specular_shininess.xyz, 1.);
  va_material_shininess = attr_specular_shininess.w;
#endif
  radius2 = radius * radius;

  vec3 rightVector = vec3(view_matrix[0][0], view_matrix[1][0], view_matrix[2][0]);

  vec3 upVector = vec3(view_matrix[0][1], view_matrix[1][1], view_matrix[2][1]);

  // corner vector
  vec3 cornerDirection = (box_correction*upFlag) * upVector + (box_correction*rightFlag) * rightVector;

  // corner vertex
  vec4 vertex = vec4(centerVertex.xyz + radius * cornerDirection, 1.0);

  // vertex position in modelview space
  vec4 eyeSpacePos = view_matrix * vertex;
  point = eyeSpacePos.xyz;

  // sphere center position in modelview space
  vec4 tmppos = view_matrix * centerVertex;
  sphere_center = tmppos.xyz;

  // Pass transformed vertex
  gl_Position = projection_view_matrix * vertex;
#if defined(HAS_CLIP_PLANE)
#if GLSL_VERSION >= 130
  for (int i=0; i<CLIP_PLANE_COUNT; ++i)
    gl_ClipDistance[i] = dot(clip_planes[i], vertex);
#else
  gl_ClipVertex = vertex;
#endif   // version 130 or up
#endif  // has clipplane
}

