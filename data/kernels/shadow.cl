const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

kernel void horizontalblur (read_only image2d_t in,
	      		    write_only image2d_t out,
			    global float *weights,
			    int num_weights)
{
	int x = get_global_id (0),
	    y = get_global_id (1);

        float sum = read_imagef (in, sampler, (int2) (x, y)).x * weights [0];
	for (int i = 1; i < num_weights; i++)
	{
		sum += (read_imagef (in, sampler, (int2) (x + i, y)).x
		        + read_imagef (in, sampler, (int2) (x - i, y)).x)
			* weights [i];
		
	}

	float4 pixel;
	pixel.x = sum;
	write_imagef (out, (int2) (x, y), pixel);
}

kernel void verticalblur (read_only image2d_t in,
	      		  write_only image2d_t out,
			  global float *weights,
			  int num_weights)
{
	int x = get_global_id (0),
	    y = get_global_id (1);

        float sum = read_imagef (in, sampler, (int2) (x, y)).x * weights [0];
	for (int i = 1; i < num_weights; i++)
	{
		sum += (read_imagef (in, sampler, (int2) (x, y + i)).x
		        + read_imagef (in, sampler, (int2) (x, y - i)).x)
			* weights [i];
		
	}

	float4 pixel;
	pixel.x = sum;
	write_imagef (out, (int2) (x, y), pixel);
}
