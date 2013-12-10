uniform float ortho;

uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;
uniform mat4 projection_matrix;

varying vec4 color;
varying vec3 sphere_center;
varying float radius2;
varying vec3 point;
#ifdef DYNAMIC_MATERIAL_PROPERTY
varying float va_material_shininess;
varying vec4 va_material_specular;
#endif

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  vec3 rayOrigin = mix(vec3(0.0 ,0.0, 0.0), point, ortho);
  vec3 rayDirection = mix(normalize(point), vec3(0.0, 0.0, -1.0), ortho);

  vec3 sphereVector = sphere_center - rayOrigin;

  // Calculate sphere-ray intersection
  float b = dot(sphereVector, rayDirection);

  float position = b * b + radius2 - dot(sphereVector, sphereVector);

  // Check if the ray missed the sphere
  if (position < 0.0)
    discard;

  float dist = b - sqrt(position);

  // point of intersection on sphere surface
  vec3 ipoint = dist * rayDirection + rayOrigin;

  // Calculate depth in clipping space
  vec2 clipZW = ipoint.z * projection_matrix[2].zw + projection_matrix[3].zw;

  float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;

  if (depth <= 0.0)
    discard;

  if (depth >= 1.0)
    discard;

  fragDepth = depth;

  vec3 normalDirection = normalize(ipoint - sphere_center);

#ifdef DYNAMIC_MATERIAL_PROPERTY
  fragColor = apply_lighting_and_fog(scene_ambient, va_material_shininess, material_ambient, va_material_specular,
                                     normalDirection, ipoint, color, alpha);
#else
  fragColor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
                                     normalDirection, ipoint, color, alpha);
#endif
    
}
