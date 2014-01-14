attribute vec3 attr_vertex;
attribute vec2 attr_2dTexCoord0;
attribute vec4 attr_color;

uniform mat4 projection_view_matrix;

varying vec2 texCoord0;
varying vec4 color;

void main() 
{
    gl_Position = projection_view_matrix * vec4(attr_vertex, 1.0);
    texCoord0 = attr_2dTexCoord0;
    color = attr_color;
}