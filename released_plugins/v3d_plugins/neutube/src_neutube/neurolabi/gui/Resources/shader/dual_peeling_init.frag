//--------------------------------------------------------------------------------------
// Order Independent Transparency with Dual Depth Peeling
//
// Author: Louis Bavoil
// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

void fragment_func(out vec4 fragColor, out float fragDepth);

void main(void)
{
	float fragDepth;
	vec4 color;
	fragment_func(color, fragDepth);
	FragData0.xy = vec2(-fragDepth, fragDepth);
	FragData1.x = -fragDepth;
}
