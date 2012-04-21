const sampler_t samplerA = CLK_NORMALIZED_COORDS_TRUE|CLK_FILTER_LINEAR
			  |CLK_ADDRESS_CLAMP_TO_EDGE;
const sampler_t samplerB = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP;

constant float3 G[9][3] = {
      {
	(float3) (0.35355339, 0.5, 0.35355339),
	(float3) (0.0, 0.0, 0.0),
	(float3) (-0.35355339, -0.5, -0.35355339),
      },
      {
	(float3) (0.35355339, 0.0, -0.35355339),
	(float3) (0.5, 0.0, -0.5),
	(float3) (0.35355339, 0.0, -0.35355339)
      },
      {
	(float3) (0, -0.35355339, 0.5),
	(float3) (0.35355339, 0.0, -0.35355339),
	(float3) (-0.5, 0.35355339, 0.0),
      },

      {
	(float3) (0.5, -0.35355339, 0.0),
	(float3) (-0.35355339, 0.0, 0.35355339),
	(float3) (0.0, 0.35355339, -0.5)
      },
      {
	(float3) (0.0, 0.5, 0.0),
	(float3) (-0.5, 0.0, -0.5),
	(float3) (0.0, 0.5, 0.0)
      },
      {
	(float3) (-0.5, 0.0, 0.5),
	(float3) (0.0, 0.0, 0.0),
	(float3) (0.5, 0.0, -0.5)
      },
      {
	(float3) (0.166666666, -0.333333333, 0.166666666),
	(float3) (-0.333333333, 0.666666666, -0.333333333),
	(float3) (0.166666666, -0.333333333, 0.166666666)
      },
      {
	(float3) (-0.333333333, 0.166666666, -0.333333333),
	(float3) (0.166666666, 0.666666666, 0.166666666),
	(float3) (-0.333333333, 0.166666666, -0.333333333),
      },
      {
	(float3) (0.333333333, 0.333333333, 0.333333333),
	(float3) (0.333333333, 0.333333333, 0.333333333),
	(float3) (0.333333333, 0.333333333, 0.333333333)
      }
};

kernel void freichen (read_only image2d_t in, write_only image2d_t out)
{
	int2 coord = (int2) (get_global_id (0), get_global_id (1));

	float3 values[3];

	for (int i = 0; i < 3; i++)
	{
		float4 sample;
		sample = read_imagef (in, samplerB,
		       	 	      coord + (int2) (i - 1, -1));
		values[i].x = length (sample);
		sample = read_imagef (in, samplerB,
		       	 	      coord + (int2) (i - 1, 0));
		values[i].y = length (sample);
		sample = read_imagef (in, samplerB,
		       	 	      coord + (int2) (i - 1, 1));
		values[i].z = length (sample);
	}

	float M = 0.0;
	float S = 0.0;

	for (int i = 0; i < 9; i++)
	{
		float tmp;
		tmp = dot (G[i][0], values[0]) +
		      dot (G[i][1], values[1]) +
		      dot (G[i][2], values[2]);
		tmp *= tmp;
		S += tmp;
		if (i < 4)
		   M += tmp;
	}
	write_imagef (out, coord, native_sqrt (native_divide (M, S)));
}

kernel void hblur (read_only image2d_t in, write_only image2d_t out,
       	    	   global float *weights, global float *offsets, int radius)
{
	int x = get_global_id (0);
	int y = get_global_id (1);

	float4 value = (float4) (0, 0, 0, 0);
	float2 coord;

	coord.y = native_divide ((float)y, (float)get_image_height (in));
	coord.x = native_divide ((float)x, (float)get_image_width (in));
	value += weights[0]
		 * read_imagef (in, samplerA, coord);
	for (int dx = 1; dx < radius; dx++)
	{
		coord.x = native_divide ((float)(x + offsets[dx]),
			  		 (float)get_image_width (in));
		if (coord.x <= 1.0)
		value += weights[dx]
		   	 * read_imagef (in, samplerA, coord);
		coord.x = native_divide ((float)(x - offsets[dx]),
			  		 (float)get_image_width (in));
		if (coord.x >= 0.0)
		value += weights[dx]
		   	 * read_imagef (in, samplerA, coord);
	}

	write_imagef (out, (int2) (x, y), value);
}

kernel void vblur (read_only image2d_t in,
       	    	   write_only image2d_t out,
       	    	   global float *weights,
		   global float *offsets,
		   int radius)
{
	int x = get_global_id (0);
	int y = get_global_id (1);

	float4 value = (float4) (0, 0, 0, 0);
	float2 coord;

	coord.x = native_divide ((float)x, (float)get_image_width (in));
	coord.y = native_divide ((float)y, (float)get_image_height (in));
	value += weights[0]
		 * read_imagef (in, samplerA, coord);

	for (int dy = 1; dy < radius; dy++)
	{
		coord.y = native_divide ((float)(y + offsets[dy]),
			  		 (float)get_image_height (in));
		if (coord.y <= 1.0)
		value += weights[dy]
		   	 * read_imagef (in, samplerA, coord);
		coord.y = native_divide ((float)(y - offsets[dy]),
			  		 (float)get_image_height (in));
		if (coord.y >= 0.0)
		value += weights[dy]
		   	 * read_imagef (in, samplerA, coord);
	}

	write_imagef (out, (int2) (x, y), value);
}
