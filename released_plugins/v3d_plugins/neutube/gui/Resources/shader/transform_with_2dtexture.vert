attribute vec3 attr_vertex;
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
attribute vec2 attr_2dTexCoord0;
#endif

uniform mat4 projection_view_matrix;

#ifndef DISABLE_TEXTURE_COORD_OUTPUT
varying vec2 texCoord0;
#endif

void main() 
{
  gl_Position = projection_view_matrix * vec4(attr_vertex, 1.0);
#ifndef DISABLE_TEXTURE_COORD_OUTPUT
  texCoord0 = attr_2dTexCoord0;
#endif
}
