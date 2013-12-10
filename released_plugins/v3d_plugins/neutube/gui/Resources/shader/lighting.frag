uniform bool lighting_enabled;

#if defined(USE_LINEAR_FOG) || defined(USE_EXPONENTIAL_FOG) || defined(USE_SQUARED_EXPONENTIAL_FOG)
uniform vec3 fog_color_top;
uniform vec3 fog_color_bottom;
#endif
#if defined(USE_LINEAR_FOG)
uniform float fog_end;
uniform float fog_scale;
#endif
#if defined(USE_EXPONENTIAL_FOG)
uniform float fog_density_log2e;
#endif
#if defined(USE_SQUARED_EXPONENTIAL_FOG)
uniform float fog_density_density_log2e;
#endif

uniform vec2 screen_dim_RCP;

struct LightSource
{
  vec4 position;
  vec4 ambient;
  vec4 diffuse;
  vec4 specular;
  vec3 attenuation;
  float spotCutoff, spotExponent;
  vec3 spotDirection;
};
uniform LightSource lights[LIGHT_COUNT];


vec4 computeColorFromLight(const in vec3 normalDirection, const in LightSource light, const in vec3 position,
                           const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                           const in vec4 color)
{
  vec3 lightDirection;
  float attenuation;
  if (0.0 == light.position.w)  // directional light
  {
    attenuation = 1.0;
    lightDirection = normalize(light.position.xyz);
  } else {    // spotlight or point light
    vec3 positionToLightSource = light.position.xyz - position;
    float distance = length(positionToLightSource);
    lightDirection = normalize(positionToLightSource);
    attenuation = 1.0 / (light.attenuation.x
                         + light.attenuation.y * distance
                         + light.attenuation.z * distance * distance);

    if (light.spotCutoff <= 90.0) // spotlight
    {
      float clampedCosine = max(0.0, dot(-lightDirection, normalize(light.spotDirection)));
      if (clampedCosine < cos(radians(light.spotCutoff))) // outside of spotlight cone
      {
        attenuation = 0.0;
      }
      else
      {
        attenuation = attenuation * pow(clampedCosine, light.spotExponent);
      }
    }
  }
  vec4 retVal = vec4(0.);
  retVal += light.ambient * materialAmbient;
  float NdotL = dot(normalDirection, lightDirection);
  if (NdotL > 0.0) {
    retVal += attenuation * light.diffuse * NdotL * color;
    vec3 cameraDirection = normalize(-position);
    float NdotH = max(dot(reflect(-lightDirection,normalDirection), cameraDirection), 0.0);
    retVal += attenuation * light.specular * materialSpecular * pow(NdotH, materialShininess);
  }
  return retVal;
}

vec4 apply_lighting_and_fog(const in vec4 sceneAmbient,
                            const in float materialShininess, const in vec4 materialAmbient, const in vec4 materialSpecular,
                            const in vec3 normalDirection, const in vec3 position, const in vec4 color, const in float alpha)
{
  if (lighting_enabled && LIGHT_COUNT > 0) {
    vec4 finalColor = sceneAmbient * materialAmbient;

    for (int index = 0; index < LIGHT_COUNT; index++) {
      finalColor += computeColorFromLight(normalDirection, lights[index], position,
                                          materialShininess, materialAmbient, materialSpecular, color);
    }

#if defined(USE_LINEAR_FOG)
    float fog = clamp((fog_end + position.z) * fog_scale, 0.0, 1.0);
    vec3 fogColor = mix(fog_color_bottom, fog_color_top, gl_FragCoord.y * screen_dim_RCP.y);
    finalColor.rgb = mix(fogColor, finalColor.rgb, fog);
#endif
#if defined(USE_EXPONENTIAL_FOG)
    float fog = clamp(exp2(position.z * fog_density_log2e), 0.0, 1.0);
    vec3 fogColor = mix(fog_color_bottom, fog_color_top, gl_FragCoord.y * screen_dim_RCP.y);
    finalColor.rgb = mix(fogColor, finalColor.rgb, fog);
#endif
#if defined(USE_SQUARED_EXPONENTIAL_FOG)
    float fog = clamp(exp2(-position.z * position.z * fog_density_density_log2e), 0.0, 1.0);
    vec3 fogColor = mix(fog_color_bottom, fog_color_top, gl_FragCoord.y * screen_dim_RCP.y);
    finalColor.rgb = mix(fogColor, finalColor.rgb, fog);
#endif

    return vec4(finalColor.rgb * color.a * alpha, color.a * alpha);
  } else {
    return color;
  }
}

