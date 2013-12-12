struct VolumeStruct
{
  sampler2D volume;
  vec3 color;
};

#ifdef VOLUME_1_EXIST
uniform VolumeStruct volume_struct_1;
#endif
#ifdef VOLUME_2_EXIST
uniform VolumeStruct volume_struct_2;
#endif
#ifdef VOLUME_3_EXIST
uniform VolumeStruct volume_struct_3;
#endif
#ifdef VOLUME_4_EXIST
uniform VolumeStruct volume_struct_4;
#endif
#ifdef VOLUME_5_EXIST
uniform VolumeStruct volume_struct_5;
#endif

varying vec2 texCoord0;

#if GLSL_VERSION < 130
#define texture texture2D
#endif

void main()
{
#if defined(VOLUME_1_EXIST) || defined(VOLUME_2_EXIST) || defined(VOLUME_3_EXIST) || defined(VOLUME_4_EXIST) || defined(VOLUME_5_EXIST)
  vec3 color = vec3(0.0);

#ifdef VOLUME_1_EXIST
  vec4 voxel1;
  voxel1 = texture(volume_struct_1.volume, texCoord0);
  color = max(color, voxel1.r * volume_struct_1.color);
#endif

#ifdef VOLUME_2_EXIST
  vec4 voxel2;
  voxel2 = texture(volume_struct_2.volume, texCoord0);
  color = max(color, voxel2.r * volume_struct_2.color);
#endif

#ifdef VOLUME_3_EXIST
  vec4 voxel3;
  voxel3 = texture(volume_struct_3.volume, texCoord0);
  color = max(color, voxel3.r * volume_struct_3.color);
#endif

#ifdef VOLUME_4_EXIST
  vec4 voxel4;
  voxel4 = texture(volume_struct_4.volume, texCoord0);
  color = max(color, voxel4.r * volume_struct_4.color);
#endif

#ifdef VOLUME_5_EXIST
  vec4 voxel5;
  voxel5 = texture(volume_struct_5.volume, texCoord0);
  color = max(color, voxel5.r * volume_struct_5.color);
#endif

  FragData0 = vec4(color, 1.0);
#else
  discard;
#endif
}