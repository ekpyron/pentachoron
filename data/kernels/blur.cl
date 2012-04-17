const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

kernel void hblur (read_only image2d_t in, write_only global image2d_t out,
       	    	   global float *weights, int radius)
{
	int x = mul24 (get_group_id (0), get_local_size (0))
	      	+ get_local_id (0);
	int y = mul24 (get_group_id (1), get_local_size (1))
	      	+ get_local_id (1);

	float4 value = (float4) (0, 0, 0, 0);

	for (int dx = -radius + 1; dx < radius; dx++)
	{
		int2 coord;
		coord = (int2) (x + dx, y);
		if (coord.x >= 0 && coord.x < get_image_width (in))
		   value += weights[abs (dx)]
		   	    * read_imagef (in, sampler, coord);
	}

	write_imagef (out, (int2) (x, y), value);
}

kernel void vblur (read_only image2d_t in,
       	    	   write_only image2d_t out,
       	    	   global float *weights, int radius)
{
	int x = mul24 (get_group_id (0), get_local_size (0))
	      	+ get_local_id (0);
	int y = mul24 (get_group_id (1), get_local_size (1))
	      	+ get_local_id (1);

	float4 value = (float4) (0, 0, 0, 0);

	for (int dy = -radius + 1; dy < radius; dy++)
	{
		int2 coord;
		coord = (int2) (x, y + dy);
		if (coord.y >= 0 && coord.y < get_image_height (out))
		   value += weights[abs (dy)]
		   	    * read_imagef (in, sampler, coord);
	}

	write_imagef (out, (int2) (x, y), value);
}
