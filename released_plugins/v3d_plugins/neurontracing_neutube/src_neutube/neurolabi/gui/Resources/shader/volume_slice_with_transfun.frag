struct VolumeStruct
{
  sampler3D volume;
};

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

varying vec3 texCoord0;

vec4 applyTF(in sampler1D tex, in vec4 intensity)
{
#if GLSL_VERSION >= 130
  return texture(tex, intensity.r);
#else
  return texture1D(tex, intensity.r);
#endif
}

void main()
{
#if defined(VOLUME_1_ACTIVE) || defined(VOLUME_2_ACTIVE) || defined(VOLUME_3_ACTIVE) || defined(VOLUME_4_ACTIVE) || defined(VOLUME_5_ACTIVE)
  vec4 color = vec4(0.0);
#ifdef VOLUME_1_ACTIVE
  vec4 voxel1;
#if GLSL_VERSION >= 130
  voxel1 = texture(volume_struct_1.volume, texCoord0);
#else
  voxel1 = texture3D(volume_struct_1.volume, texCoord0);
#endif
  vec4 color1 = applyTF(transfer_function_1, voxel1);

  if (color1.a > 0.0) {
    color = max(color, color1);
  }
#endif

#ifdef VOLUME_2_ACTIVE
  vec4 voxel2;
#if GLSL_VERSION >= 130
  voxel2 = texture(volume_struct_2.volume, texCoord0);
#else
  voxel2 = texture3D(volume_struct_2.volume, texCoord0);
#endif
  vec4 color2 = applyTF(transfer_function_2, voxel2);

  if (color2.a > 0.0) {
    color = max(color, color2);
  }
#endif

#ifdef VOLUME_3_ACTIVE
  vec4 voxel3;
#if GLSL_VERSION >= 130
  voxel3 = texture(volume_struct_3.volume, texCoord0);
#else
  voxel3 = texture3D(volume_struct_3.volume, texCoord0);
#endif
  vec4 color3 = applyTF(transfer_function_3, voxel3);

  if (color3.a > 0.0) {
    color = max(color, color3);
  }
#endif

#ifdef VOLUME_4_ACTIVE
  vec4 voxel4;
#if GLSL_VERSION >= 130
  voxel4 = texture(volume_struct_4.volume, texCoord0);
#else
  voxel4 = texture3D(volume_struct_4.volume, texCoord0);
#endif
  vec4 color4 = applyTF(transfer_function_4, voxel4);

  if (color4.a > 0.0) {
    color = max(color, color4);
  }
#endif

#ifdef VOLUME_5_ACTIVE
  vec4 voxel5;
#if GLSL_VERSION >= 130
  voxel5 = texture(volume_struct_5.volume, texCoord0);
#else
  voxel5 = texture3D(volume_struct_5.volume, texCoord0);
#endif
  vec4 color5 = applyTF(transfer_function_5, voxel5);

  if (color5.a > 0.0) {
    color = max(color, color5);
  }
#endif

#ifdef RESULT_OPAQUE
  color.a = 1.0;
#else
  if (color.a == 0.0)
    discard;
#endif

  color.rgb *= color.a;
  FragData0 = color;
#else
  discard;
#endif
}