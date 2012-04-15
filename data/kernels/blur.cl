const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

kernel void hblur (read_only image2d_t in, write_only global float4 *out,
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
		else
		   value += weights[abs (dx)] * (float4) (1.0, 1.0, 1.0, 1.0);
	}

	out[get_image_width (in) * y + x] = value;
}

kernel void vblur (read_only global float4 *in,
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
		if (y+dy >= 0 && y+dy < get_image_height (out))
		{
		   value += weights[abs (dy)]
		   	    * in[(y + dy) * get_image_width (out) + x];
		}
		else
		   value += weights[abs (dy)] * (float4) (1.0, 1.0, 1.0, 1.0);
	}

	write_imagef (out, (int2) (x, y), value);
}
