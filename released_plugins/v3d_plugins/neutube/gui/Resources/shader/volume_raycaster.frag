struct VolumeStruct
{
  sampler3D volume;
  vec3 dimensions;
};

uniform vec2 screen_dim_RCP;
uniform float sampling_rate;
#ifdef ISO
uniform float iso_value;
#endif
#ifdef LOCAL_MIP
uniform float local_MIP_threshold;
#endif
uniform float ze_to_zw_a;
uniform float ze_to_zw_b;

uniform sampler2D ray_entry_points;
uniform sampler2D ray_entry_points_depth;
uniform sampler2D ray_exit_points;
uniform sampler2D ray_exit_points_depth;

#ifdef VOLUME_1_ACTIVE
uniform VolumeStruct volume_struct_1;
uniform TF_SAMPLER_TYPE_1 transfer_function_1;
#endif

#ifdef VOLUME_2_ACTIVE
uniform VolumeStruct volume_struct_2;
uniform TF_SAMPLER_TYPE_2 transfer_function_2;
#endif

#ifdef VOLUME_3_ACTIVE
uniform VolumeStruct volume_struct_3;
uniform TF_SAMPLER_TYPE_3 transfer_function_3;
#endif

#ifdef VOLUME_4_ACTIVE
uniform VolumeStruct volume_struct_4;
uniform TF_SAMPLER_TYPE_4 transfer_function_4;
#endif

#ifdef VOLUME_5_ACTIVE
uniform VolumeStruct volume_struct_5;
uniform TF_SAMPLER_TYPE_5 transfer_function_5;
#endif

vec4 applyTF(in sampler1D tex, in float intensity)
{
#if GLSL_VERSION >= 130
#if defined(MIP)
  return texture(tex, intensity);
#else
  vec4 res = texture(tex, intensity);
  res.a = res.a / sampling_rate;
  return res;
#endif
#else
#if defined(MIP)
  return texture1D(tex, intensity);
#else
  vec4 res = texture1D(tex, intensity);
  res.a = res.a / sampling_rate;
  return res;
#endif
#endif
}

vec4 applyTF(in float ch1V, in float ch2V, in float ch3V, in float ch4V, in float ch5V)
{
  vec4 res = vec4(0,0,0,0);
#ifdef VOLUME_1_ACTIVE
  res = max(res, applyTF(transfer_function_1, ch1V));
#endif

#ifdef VOLUME_2_ACTIVE
  res = max(res, applyTF(transfer_function_2, ch2V));
#endif

#ifdef VOLUME_3_ACTIVE
  res = max(res, applyTF(transfer_function_3, ch3V));
#endif

#ifdef VOLUME_4_ACTIVE
  res = max(res, applyTF(transfer_function_4, ch4V));
#endif

#ifdef VOLUME_5_ACTIVE
  res = max(res, applyTF(transfer_function_5, ch5V));
#endif

  return res;
}

vec4 compositeDVR(in vec4 curResult, in vec4 color, in float currentRayLength, inout float rayDepth)
{
  if (rayDepth < 0.0)
    rayDepth = currentRayLength;

  vec4 result = vec4(0.0);

  result.a = curResult.a + (1.0 -curResult.a) * color.a;
  result.rgb = (curResult.rgb * curResult.a + (1.0 - curResult.a) * color.a * color.rgb) / result.a;

  return result;
}

vec4 compositeISO(in vec4 curResult, in vec4 color, in float currentRayLength, inout float rayDepth, in float isoValue)
{
  vec4 result = curResult;
  float epsilon = 0.02;
  if (color.a >= isoValue-epsilon && color.a <= isoValue+epsilon) {
    result = color;
    result.a = 1.0;
    rayDepth = currentRayLength;
  }
  return result;
}

vec4 compositeXRay(in vec4 curResult, in vec4 color, in float currentRayLength, inout float rayDepth)
{
  if (rayDepth < 0.0)
    rayDepth = currentRayLength;
  return curResult + color;
}

void main()
{
#if defined(VOLUME_1_ACTIVE) || defined(VOLUME_2_ACTIVE) || defined(VOLUME_3_ACTIVE) || defined(VOLUME_4_ACTIVE) || defined(VOLUME_5_ACTIVE)
  vec2 texCoords = gl_FragCoord.xy * screen_dim_RCP;
#if GLSL_VERSION >= 130
  vec3 startRayPosition = texture(ray_entry_points, texCoords).xyz;
  vec3 exitRayPosition = texture(ray_exit_points, texCoords).xyz;
#else
  vec3 startRayPosition = texture2D(ray_entry_points, texCoords).xyz;
  vec3 exitRayPosition = texture2D(ray_exit_points, texCoords).xyz;
#endif

  if (startRayPosition == exitRayPosition)
    discard;   // background
  else {
    vec4 result = vec4(0.0);

#ifdef MIP
    float ch1V = 0.0;
    float ch2V = 0.0;
    float ch3V = 0.0;
    float ch4V = 0.0;
    float ch5V = 0.0;
#endif

#ifdef LOCAL_MIP
    bool ch1Done = false;
    bool ch2Done = false;
    bool ch3Done = false;
    bool ch4Done = false;
    bool ch5Done = false;
#endif

#ifdef VOLUME_1_ACTIVE
    vec3 dimension = volume_struct_1.dimensions;
#elif defined(VOLUME_2_ACTIVE)
    vec3 dimension = volume_struct_2.dimensions;
#elif defined(VOLUME_3_ACTIVE)
    vec3 dimension = volume_struct_3.dimensions;
#elif defined(VOLUME_4_ACTIVE)
    vec3 dimension = volume_struct_4.dimensions;
#elif defined(VOLUME_5_ACTIVE)
    vec3 dimension = volume_struct_5.dimensions;
#endif
    vec3 rayVector = exitRayPosition - startRayPosition;
    float maxRayLength = length(rayVector);
    float stepSize = maxRayLength / (sampling_rate * length(normalize(rayVector) * dimension));

    float currentRayLength = 0.0;
    float rayDepth = -1.0;
    bool finished = false;
    for (int loop0=0; !finished && loop0<255; loop0++) {
      for (int loop1=0; !finished && loop1<255; loop1++) {
        vec3 voxel = vec3(0.0);
        vec4 color = vec4(0.0);
        vec3 samplePos = startRayPosition + currentRayLength / maxRayLength * rayVector;
        bool saturated = true;

#ifdef VOLUME_1_ACTIVE
        float voxel1;

#if GLSL_VERSION >= 130
        voxel1 = texture(volume_struct_1.volume, samplePos).r;
#else
        voxel1 = texture3D(volume_struct_1.volume, samplePos).r;
#endif

#ifdef MIP
#ifdef LOCAL_MIP
        if (!ch1Done) {
          if (voxel1 <= ch1V && ch1V >= local_MIP_threshold) {
            ch1Done = true;
          } else if (voxel1 > ch1V) {
            ch1V = voxel1;
            rayDepth = currentRayLength;
          }
        }
        saturated = saturated && ch1Done;
#else
        if (voxel1 > ch1V) {
          rayDepth = currentRayLength;
          ch1V = voxel1;
        }
        saturated = saturated && ch1V >= 1.0;
#endif
#else
        vec4 color1 = applyTF(transfer_function_1, voxel1);

        if (color1.a > 0.0) {
          color = max(color, color1);
        }
#endif //MIP

#endif


#ifdef VOLUME_2_ACTIVE
        float voxel2;

#if GLSL_VERSION >= 130
        voxel2 = texture(volume_struct_2.volume, samplePos).r;
#else
        voxel2 = texture3D(volume_struct_2.volume, samplePos).r;
#endif

#ifdef MIP
#ifdef LOCAL_MIP
        if (!ch2Done) {
          if (voxel2 <= ch2V && ch2V >= local_MIP_threshold) {
            ch2Done = true;
          } else if (voxel2 > ch2V) {
            ch2V = voxel2;
            rayDepth = currentRayLength;
          }
        }
        saturated = saturated && ch2Done;
#else
        if (voxel2 > ch2V) {
          rayDepth = currentRayLength;
          ch2V = voxel2;
        }
        saturated = saturated && ch2V >= 1.0;
#endif
#else
        vec4 color2 = applyTF(transfer_function_2, voxel2);

        if (color2.a > 0.0) {
          color = max(color, color2);
        }
#endif //MIP

#endif


#ifdef VOLUME_3_ACTIVE
        float voxel3;

#if GLSL_VERSION >= 130
        voxel3 = texture(volume_struct_3.volume, samplePos).r;
#else
        voxel3 = texture3D(volume_struct_3.volume, samplePos).r;
#endif

#ifdef MIP
#ifdef LOCAL_MIP
        if (!ch3Done) {
          if (voxel3 <= ch3V && ch3V >= local_MIP_threshold) {
            ch3Done = true;
          } else if (voxel3 > ch3V) {
            ch3V = voxel3;
            rayDepth = currentRayLength;
          }
        }
        saturated = saturated && ch3Done;
#else
        if (voxel3 > ch3V) {
          rayDepth = currentRayLength;
          ch3V = voxel3;
        }
        saturated = saturated && ch3V >= 1.0;
#endif
#else
        vec4 color3 = applyTF(transfer_function_3, voxel3);

        if (color3.a > 0.0) {
          color = max(color, color3);
        }
#endif //MIP

#endif



#ifdef VOLUME_4_ACTIVE
        float voxel4;

#if GLSL_VERSION >= 130
        voxel4 = texture(volume_struct_4.volume, samplePos).r;
#else
        voxel4 = texture3D(volume_struct_4.volume, samplePos).r;
#endif

#ifdef MIP
#ifdef LOCAL_MIP
        if (!ch4Done) {
          if (voxel4 <= ch4V && ch4V >= local_MIP_threshold) {
            ch4Done = true;
          } else if (voxel4 > ch4V) {
            ch4V = voxel4;
            rayDepth = currentRayLength;
          }
        }
        saturated = saturated && ch4Done;
#else
        if (voxel4 > ch4V) {
          rayDepth = currentRayLength;
          ch4V = voxel4;
        }
        saturated = saturated && ch4V >= 1.0;
#endif
#else
        vec4 color4 = applyTF(transfer_function_4, voxel4);

        if (color4.a > 0.0) {
          color = max(color, color4);
        }
#endif //MIP

#endif

#ifdef VOLUME_5_ACTIVE
        float voxel5;

#if GLSL_VERSION >= 130
        voxel5 = texture(volume_struct_5.volume, samplePos).r;
#else
        voxel5 = texture3D(volume_struct_5.volume, samplePos).r;
#endif

#ifdef MIP
#ifdef LOCAL_MIP
        if (!ch5Done) {
          if (voxel5 <= ch5V && ch5V >= local_MIP_threshold) {
            ch5Done = true;
          } else if (voxel5 > ch5V) {
            ch5V = voxel5;
            rayDepth = currentRayLength;
          }
        }
        saturated = saturated && ch5Done;
#else
        if (voxel5 > ch5V) {
          rayDepth = currentRayLength;
          ch5V = voxel5;
        }
        saturated = saturated && ch5V >= 1.0;
#endif
#else
        vec4 color5 = applyTF(transfer_function_5, voxel5);

        if (color5.a > 0.0) {
          color = max(color, color5);
        }
#endif //MIP

#endif



#ifdef MIP
        finished = saturated;
#else
        if (color.a > 0.0) {
          result = COMPOSITING(result, color, currentRayLength, rayDepth);
        }

        if (result.a >= 1.0) {
          result.a = 1.0;
          finished = true;
        }
#endif // MIP
        currentRayLength += stepSize;
        finished = finished || (currentRayLength > maxRayLength);
      }
    }

#ifdef MIP
    result = applyTF(ch1V, ch2V, ch3V, ch4V, ch5V);
#endif // MIP

#ifdef RESULT_OPAQUE
    result.a = 1.0;
#endif


    if (rayDepth >= 0.0) {
      //http://www.opengl.org/archives/resources/faq/technical/depthbuffer.htm
      // zw = a/ze + b;  ze = a/(zw - b);  a = f*n/(f-n);  b = 0.5*(f+n)/(f-n) + 0.5;
#if GLSL_VERSION >= 130
      float zwFront = texture(ray_entry_points_depth, texCoords).z;
      float zwBack = texture(ray_exit_points_depth, texCoords).z;
#else
      float zwFront = texture2D(ray_entry_points_depth, texCoords).z;
      float zwBack = texture2D(ray_exit_points_depth, texCoords).z;
#endif
      float zeFront = ze_to_zw_a / (zwFront - ze_to_zw_b);
      float zeBack = ze_to_zw_a / (zwBack - ze_to_zw_b);
      float ze = zeFront + rayDepth / maxRayLength * (zeBack-zeFront);
      gl_FragDepth = ze_to_zw_a / ze + ze_to_zw_b;
    } else {
#ifdef RESULT_OPAQUE
#if GLSL_VERSION >= 130
      gl_FragDepth = texture(ray_entry_points_depth, texCoords).z;
#else
      gl_FragDepth = texture2D(ray_entry_points_depth, texCoords).z;
#endif
#else
      gl_FragDepth = 1.0;
#endif
    }

    result.rgb *= result.a;
    FragData0 = result;
  }
#endif
}
