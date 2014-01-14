attribute vec3 attr_vertex;
attribute vec2 attr_2dTexCoord0;

varying vec2 texCoord0;

void main() 
{
    gl_Position = vec4(attr_vertex, 1.0);
    texCoord0 = attr_2dTexCoord0;
}
