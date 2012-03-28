#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics: enable

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

constant float4 small4 = (float4) (0.001, 0.001, 0.001, 0.001);

struct Light
{
	union {
	      struct {
	      	     float4 position;
		     float4 color;
		     float4 direction;
		     float4 spot;
		     float4 specular;
		     float4 attenuation;
	      };
	      float4 data[6];
	};
};

struct ViewInfo
{
	float4 projinfo;
	float4 vmatinv[4];
	float4 eye;
};

float3 reflect (float3 I, float3 N)
{
	return mad (-2.0 * dot (N, I), N, I);
}

float4 getpos (uint x, uint y, read_only image2d_t depthbuffer,
               struct ViewInfo info)
{
	float depth = read_imagef (depthbuffer, sampler, (int2) (x, y)).x;
	float4 pos;
	pos.x = native_divide ((float)x, (float)get_image_width (depthbuffer));
	pos.y = native_divide ((float)y, (float)get_image_height (depthbuffer));
	pos.z = depth;
	pos.w = 1;
	pos.xyz = mad (pos.xyz, 2, -1);

	float4 p;
	p.x = pos.x * info.projinfo.x;
	p.y = pos.y * info.projinfo.y;
	p.z = -pos.w;
	p.w = native_divide (pos.z * (info.projinfo.z - info.projinfo.w)
	      		     + pos.w * (info.projinfo.z + info.projinfo.w),
			     2 * info.projinfo.z * info.projinfo.w);

	pos.x = dot (info.vmatinv[0], p);
	pos.y = dot (info.vmatinv[1], p);
	pos.z = dot (info.vmatinv[2], p);
	pos.w = dot (info.vmatinv[3], p);
	pos.xyz = native_divide (pos.xyz, pos.w);

	return pos;
}

float4 compute_pixel (read_only image2d_t colormap, read_only image2d_t depthbuffer, read_only image2d_t normalmap, read_only image2d_t specularmap,
       	        float shadow, uint offset, uint x, uint y, unsigned int num_lights, global struct Light *lights, struct ViewInfo info)
{
	float4 pos = getpos (x, y, depthbuffer, info);
	float3 diffuse = (float3) (0, 0, 0);
	float3 specular = (float3) (0, 0, 0);
	local ushort light_indices[256];
	local uint num_light_indices;

	local float min_depth, max_depth;

	min_depth = FLT_MAX;
	max_depth = FLT_MIN;

/*	barrier (CLK_LOCAL_MEM_FENCE);

	// TODO: maybe atomic operations are in fact necessary
	min_depth = min (min_depth, depth);
	max_depth = max (max_depth, depth);*/

	num_light_indices = 0;

	barrier (CLK_LOCAL_MEM_FENCE);

	if (offset < num_lights)
	{
		// TODO: Actual light culling is needed here
		ulong light = atom_inc (&num_light_indices);
		light_indices[light] = offset;
	}


	barrier (CLK_LOCAL_MEM_FENCE);

	for (int i = 0; i < num_light_indices; i++)
	{
		struct Light light;
		light = lights[light_indices[i]];
		float3 lightDir = light.position.xyz - pos.xyz;

		float dist = fast_length (lightDir);
		lightDir = native_divide (lightDir, dist);

		float attenuation;
		attenuation = native_recip (mad (light.attenuation.z,
		      		       dist * dist,
		     		       mad (light.attenuation.y,
					    dist, light.attenuation.x)));
		if (attenuation < 0.001)
		   continue;

		float spotEffect;
		
		spotEffect = dot (fast_normalize (light.direction.xyz),
			     	  -lightDir);
		if (spotEffect < light.spot.x)
		   continue;
		spotEffect = native_powr (spotEffect, light.spot.y);

		attenuation *= spotEffect;
		if (attenuation < 0.001)
		   continue;
		
		float3 normal = mad (read_imagef (normalmap, sampler,
		       	  	      	          (int2) (x, y)).xyz, 2, -1);

		float NdotL;

		NdotL = max (dot (normal, lightDir), 0.0f);
		
		diffuse += attenuation * NdotL * light.color.xyz;

		float r;
		r = dot (fast_normalize (info.eye.xyz - pos.xyz),
		    	 reflect (fast_normalize (pos.xyz
			 	  - light.position.xyz),
			 	  normal));

		specular += attenuation * light.specular.xyz
			    * native_powr (r, light.specular.w);
	}

	diffuse *= shadow;
	specular *= shadow;

	diffuse += 0.1;

	float4 pixel = read_imagef (colormap, sampler, (int2) (x, y));

	pixel.xyz *= diffuse;

	if (any (specular > small4))
	{
		specular *= read_imagef (specularmap, sampler,
			    		 (int2) (x, y)).xyz;
		pixel.xyz += specular;
	}

	return pixel;
}

kernel void composition (write_only image2d_t screen,
	      		 read_only image2d_t colormap,
	      		 read_only image2d_t colormap2,
	      		 read_only image2d_t colormap3,
			 read_only image2d_t depthbuffer,
			 read_only image2d_t depthbuffer2,
			 read_only image2d_t depthbuffer3,
			 read_only image2d_t normalmap,
			 read_only image2d_t normalmap2,
			 read_only image2d_t normalmap3,
			 read_only image2d_t specularmap,
			 read_only image2d_t specularmap2,
			 read_only image2d_t specularmap3,
			 read_only image2d_t shadowmask,
			 unsigned int num_lights,
			 global struct Light *lights,
			 struct ViewInfo info)
{
	uint lx = get_local_id (0),
	    ly = get_local_id (1);
	uint x = mad24 (get_group_id (0), get_local_size (0), lx),
	    y = mad24 (get_group_id (1), get_local_size (1), ly);
	uint offset = mad24 (ly, get_local_size (1), lx);
	float shadow = read_imagef (shadowmask, sampler, (int2) (x, y)).x;

	float4 pixel;

	pixel = compute_pixel (colormap, depthbuffer, normalmap,
	      		       specularmap, shadow, offset,
			       x, y, num_lights, lights, info);
	if (pixel.w < 0.99)
	{
		float4 pixel2;
		pixel2 = compute_pixel (colormap2, depthbuffer2,
		       	 	        normalmap2, specularmap2,
					shadow, offset, x, y,
					num_lights, lights, info);
		pixel.xyz = mix (pixel2.xyz, pixel.xyz, pixel.w);
		if (pixel.w < 0.99)
		{
			pixel2.xyz = compute_pixel (colormap3, depthbuffer3,
			       	 	        normalmap3, specularmap3,
						shadow, offset, x, y,
						num_lights, lights, info).xyz;
			pixel.xyz = mix (pixel2.xyz, pixel.xyz, pixel2.w);
		}
	}

	write_imagef (screen, (int2) (x, y), pixel);
}
