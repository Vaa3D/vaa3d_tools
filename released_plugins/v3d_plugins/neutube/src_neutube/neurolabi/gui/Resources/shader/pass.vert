attribute vec3 attr_vertex;

void main() 
{
    gl_Position = vec4(attr_vertex, 1.0);
}
