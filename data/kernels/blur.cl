const sampler_t sampler = CLK_NORMALIZED_COORDS_TRUE|CLK_FILTER_LINEAR
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

kernel void hblur (read_only image2d_t in, write_only global image2d_t out,
       	    	   global float *weights, global float *offsets, int radius)
{
	int x = get_global_id (0);
	int y = get_global_id (1);

	float4 value = (float4) (0, 0, 0, 0);
	float2 coord;

	coord.y = native_divide ((float)y, (float)get_image_height (in));
	coord.x = native_divide ((float)x, (float)get_image_width (in));
	value += weights[0]
		 * read_imagef (in, sampler, coord);
	for (int dx = 1; dx < radius; dx++)
	{
		coord.x = native_divide ((float)(x + offsets[dx]),
			  		 (float)get_image_width (in));
		if (coord.x <= 1.0)
		value += weights[dx]
		   	 * read_imagef (in, sampler, coord);
		coord.x = native_divide ((float)(x - offsets[dx]),
			  		 (float)get_image_width (in));
		if (coord.x >= 0.0)
		value += weights[dx]
		   	 * read_imagef (in, sampler, coord);
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
		 * read_imagef (in, sampler, coord);

	for (int dy = 1; dy < radius; dy++)
	{
		coord.y = native_divide ((float)(y + offsets[dy]),
			  		 (float)get_image_height (in));
		if (coord.y <= 1.0)
		value += weights[dy]
		   	 * read_imagef (in, sampler, coord);
		coord.y = native_divide ((float)(y - offsets[dy]),
			  		 (float)get_image_height (in));
		if (coord.y >= 0.0)
		value += weights[dy]
		   	 * read_imagef (in, sampler, coord);
	}

	write_imagef (out, (int2) (x, y), value);
}
