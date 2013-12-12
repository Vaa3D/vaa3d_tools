// About Round Base Cap: based on view angle, if base radius is bigger than tip to base distance, the 
// round cap might be clipped because we only keep the pixels in cone. Draw another sphere might be 
// a better solution.
// Round Top Cap should be ok

uniform float ortho;

uniform vec4 scene_ambient;
uniform vec4 material_ambient;
uniform vec4 material_specular;
uniform float material_shininess;
uniform float alpha;
uniform mat4 projection_matrix;

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

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha);

void fragment_func(out vec4 fragColor, out float fragDepth)
{
  vec3 rayOrigin = mix(vec3(0.0), point, ortho);
  vec3 rayDirection = mix(normalize(point), vec3(0.0, 0.0, -1.0), ortho);

  // http://www.cl.cam.ac.uk/teaching/2000/AGraphHCI/SMEG/node2.html
  // basis is local system of coordinates for the cone
  mat3 basis = mat3(U, V, axis);
  vec3 D = rayDirection * basis;

  float bradius2 = bradius*bradius;
  float tradius2 = tradius*tradius;
  vec3 coneTip = base - axis * bradius * height / (tradius - bradius);

  float a0, a1, a2;
  if (tradius - bradius < 0.001)  {   // cylinder
    vec3 diff = point - base;
    vec3 P = diff * basis;
    a0 = P.x*P.x + P.y*P.y - bradius2;
    a1 = P.x*D.x + P.y*D.y;
    a2 = D.x*D.x + D.y*D.y;
  } else {  //cone
    vec3 diff = point - coneTip;
    vec3 P = diff * basis;
    vec3 factor = vec3(1.0, 1.0, - (tradius-bradius) * (tradius-bradius) * inv_sqr_height);
    a0 = dot(P * P, factor);
    a1 = dot(P * D, factor);
    a2 = dot(D * D, factor);
  }

  float d = a1*a1 - a0*a2;
  if (d < 0.0)
    // outside of the double cone
    discard;

  float dist = (-a1 - sqrt(d))/a2;

  // point of intersection on cone surface
  vec3 ipoint = point + dist * rayDirection;

  vec3 tmpPoint = ipoint - base;
  //vec3 normalDirection = normalize(tmpPoint - axis * dot(tmpPoint, axis));
  vec3 planeNormalDirection = cross(axis, tmpPoint);
  vec3 centerToSurfaceDirection = normalize(cross(planeNormalDirection, axis));
  vec3 baseEdge = base + centerToSurfaceDirection * bradius;
  vec3 topEdge = top + centerToSurfaceDirection * tradius;
  vec3 normalDirection = normalize(cross(planeNormalDirection, topEdge - baseEdge));

  float ratio = dot(ipoint-base, vec3(top-base)) * inv_sqr_height;

  vec4 color = mix(color1, color2, ratio);

  // test top cap
  float capTest = dot((ipoint - top), axis);

#if defined(NO_CAPS)
  if (capTest > 0.0 || dot(rayDirection, -axis) > dot(normalize(baseEdge-topEdge), -axis)) {  // render next intersection point
    dist = (-a1 + sqrt(d))/a2;
    ipoint = point + dist * rayDirection;
    if (dot(ipoint-base, axis) < 0.0 || dot(ipoint-top, axis) > 0.0)
      discard;

    tmpPoint = ipoint - base;
    planeNormalDirection = cross(axis, tmpPoint);
    centerToSurfaceDirection = normalize(cross(planeNormalDirection, axis));
    baseEdge = base + centerToSurfaceDirection * bradius;
    topEdge = top + centerToSurfaceDirection * tradius;
    normalDirection = normalize(cross(planeNormalDirection, topEdge - baseEdge));
    ratio = dot(ipoint-base, vec3(top-base)) * inv_sqr_height;
    color = mix(color1, color2, ratio);
  }
#endif

#if defined(FLAT_CAPS)
  if (capTest > 0.0 || dot(rayDirection, -axis) > dot(normalize(baseEdge-topEdge), -axis)) {
    ipoint = ipoint + rayDirection * 
      length(top - ipoint) * dot(normalize(top - ipoint), -axis) / dot(rayDirection, -axis);
    if (dot(ipoint-top, ipoint-top) > tradius2)
      discard;
    
    color = color2;
    normalDirection = axis;
  }
#endif

#if defined(ROUND_BASE_CAP_FLAT_TOP_CAP)
  if (capTest > 0.0 || dot(rayDirection, -axis) > dot(normalize(baseEdge-topEdge), -axis)) {
    // ray-plane intersection
    color = color2;
    float dNV = dot(-axis, rayDirection);
    if (dNV < 0.0) discard;
    float dist = dot(-axis, top-rayOrigin) / dNV;
    ipoint = rayDirection * dist + rayOrigin;
    // within the cap radius?
    if (dot(ipoint-top, ipoint-top) > tradius2) 
      discard;
    normalDirection = axis;
  }
#endif

#if defined(ROUND_CAPS) || defined(FLAT_BASE_CAP_ROUND_TOP_CAP)
  if (capTest > 0.0 || dot(rayDirection, -axis) > dot(normalize(baseEdge-topEdge), -axis)) {
    color = color2;
    vec3 sphereVector = top - rayOrigin;
    float b = dot(sphereVector, rayDirection);
    float pos = b*b + tradius2 -dot(sphereVector, sphereVector);
    if ( pos < 0.0)
      discard;
    float dist = b-sqrt(pos);
    ipoint = dist * rayDirection + rayOrigin;
    normalDirection = normalize( ipoint - top );
  }
#endif

  // test base cap
  capTest = dot((ipoint - base), axis);

#if defined(NO_CAPS)
  if (capTest < 0.0) {  // render next intersection point
    dist = (-a1 + sqrt(d))/a2;
    ipoint = point + dist * rayDirection;
    if (dot(ipoint-base, axis) < 0.0 || dot(ipoint-top, axis) > 0.0)
      discard;

    tmpPoint = ipoint - base;
    planeNormalDirection = cross(axis, tmpPoint);
    centerToSurfaceDirection = normalize(cross(planeNormalDirection, axis));
    baseEdge = base + centerToSurfaceDirection * bradius;
    topEdge = top + centerToSurfaceDirection * tradius;
    normalDirection = normalize(cross(planeNormalDirection, topEdge - baseEdge));
    ratio = dot(ipoint-base, vec3(top-base)) * inv_sqr_height;
    color = mix(color1, color2, ratio);
  }
#endif

#if defined(FLAT_CAPS)
  if (capTest < 0.0) {
    ipoint = ipoint + rayDirection * 
      length(base - ipoint) * dot(normalize(base - ipoint), -axis) / dot(rayDirection, -axis);
    if (dot(ipoint-base, ipoint-base) > bradius2)
      discard;
    
    color = color1;
    normalDirection = -axis;
  }
#endif

#if defined(FLAT_BASE_CAP_ROUND_TOP_CAP)
  if (capTest < 0.0) {
    // ray-plane intersection
    color = color1;
    float dNV = dot(axis, rayDirection);
    if (dNV < 0.0) discard;
    float dist = dot(axis, base-rayOrigin) / dNV;
    ipoint = rayDirection * dist + rayOrigin;
    // within the cap radius?
    if (dot(ipoint-base, ipoint-base) > bradius2) 
      discard;
    normalDirection = -axis;
  }
#endif

#if defined(ROUND_CAPS) || defined(ROUND_BASE_CAP_FLAT_TOP_CAP)
  if (capTest < 0.0) {
    if (dot(ipoint - coneTip, axis) < 0.0)
      discard;
    color = color1;
    vec3 sphereVector = base - rayOrigin;
    float b = dot(sphereVector, rayDirection);
    float pos = b*b + bradius2 -dot(sphereVector, sphereVector);
    if ( pos < 0.0)
      discard;
    float dist = b-sqrt(pos);
    ipoint = dist * rayDirection + rayOrigin;
    normalDirection = normalize( ipoint - base );
  }
#endif

  vec2 clipZW = ipoint.z * projection_matrix[2].zw + projection_matrix[3].zw;

  float depth = 0.5 + 0.5 * clipZW.x / clipZW.y;

  if (depth <= 0.0)
    discard;

  if (depth >= 1.0)
    discard;

  fragDepth = depth;

  fragColor = apply_lighting_and_fog(scene_ambient, material_shininess, material_ambient, material_specular,
                                     normalDirection, ipoint, color, alpha);

}

