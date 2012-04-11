#pragma OPENCL EXTENSION cl_khr_int64_base_atomics: enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics: enable

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;
const sampler_t samplerB = CLK_NORMALIZED_COORDS_TRUE|CLK_FILTER_LINEAR
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
	float4 shadowmat[4];
	float4 eye;
};

float3 reflect (float3 I, float3 N)
{
	return mad (-2.0 * dot (N, I), N, I);
}

float getpos (float4 *pos, struct ViewInfo *info)
{
	float depth = pos->z;
	if (depth == 1.0)
	   return depth;
	pos->w = 1;
	pos->xyz = mad (pos->xyz, 2, -1);

	float4 p;
	p.x = pos->x * info->projinfo.x;
	p.y = pos->y * info->projinfo.y;
	p.z = -pos->w;
	p.w = native_divide (pos->z * (info->projinfo.z - info->projinfo.w)
	      		     + pos->w * (info->projinfo.z + info->projinfo.w),
			     2 * info->projinfo.z * info->projinfo.w);

	pos->x = dot (info->vmatinv[0], p);
	pos->y = dot (info->vmatinv[1], p);
	pos->z = dot (info->vmatinv[2], p);
	pos->w = dot (info->vmatinv[3], p);
	pos->xyz = native_divide (pos->xyz, pos->w);
	pos->w = 1.0;

	return depth;
}

float compute_shadow (int x, int y, float4 pos,
      		      read_only image2d_t shadowmap, struct ViewInfo *info)
{
	float4 lspos;

	lspos.x = dot (info->shadowmat[0], pos);
	lspos.y = dot (info->shadowmat[1], pos);
	lspos.z = dot (info->shadowmat[2], pos);
	lspos.w = dot (info->shadowmat[3], pos);
	lspos.xyz = native_divide (lspos.xyz, lspos.w);

	if (lspos.w < 0 || lspos.x < 0 || lspos.y < 0
	    || lspos.x > 1 || lspos.y > 1)
	{
	    return 1.0;
	}

	float2 moments;
	moments = read_imagef (shadowmap, samplerB, lspos.xy).xy;

	if (lspos.z <= moments.x)
	   return 1;

	float variance = moments.y - (moments.x * moments.x);
	variance = max (variance, 0.001);//info.min_variance);
	float d = lspos.z - moments.x;
	return native_divide (variance, variance + d * d);
}

float4 compute_pixel (read_only image2d_t colormap,
       		      float4 p,
		      read_only image2d_t normalmap,
		      read_only image2d_t specularmap,
		      read_only image2d_t shadowmap,
		      uint lx, uint ly, uint x, uint y,
		      global struct Light *lights,
		      unsigned int num_light_indices,
		      local ushort *light_indices,
		      struct ViewInfo *info)
{
	float4 pos = p;
	float depth = getpos (&pos, info);
	if (depth == 1.0)
	   return (float4) (0.0, 0.0, 0.0, 1.0);
	float3 diffuse = (float3) (0, 0, 0);
	float3 specular = (float3) (0, 0, 0);
	uint offset = mad24 (ly, get_local_size (0), lx);

	for (int i = 0; i < num_light_indices; i++)
	{
		struct Light light;
		light = lights[light_indices[i]];
		float3 lightDir = light.position.xyz - pos.xyz;

		float dist = fast_length (lightDir);
		lightDir = native_divide (lightDir, dist);

		float attenuation;
		attenuation = native_recip (mad (light.attenuation.z,
			    dist * dist, mad (light.attenuation.y,
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
		r = dot (fast_normalize (info->eye.xyz - pos.xyz),
		    	 reflect (fast_normalize (pos.xyz
			 	  - light.position.xyz),
			 	  normal));

		if (light.specular.w != 0.0)
		{
			specular += attenuation * light.specular.xyz
				     * native_powr (r, light.specular.w);
		}
	}

	float shadow;
	shadow = compute_shadow (x, y, pos, shadowmap, info);

	diffuse *= shadow;
	specular *= shadow;

	diffuse += 0.05;

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
	      		 read_only image2d_t colormap1,
	      		 read_only image2d_t colormap2,
	      		 read_only image2d_t colormap3,
	      		 read_only image2d_t colormap4,
			 read_only image2d_t depthbuffer1,
			 read_only image2d_t depthbuffer2,
			 read_only image2d_t depthbuffer3,
			 read_only image2d_t depthbuffer4,
			 read_only image2d_t normalmap1,
			 read_only image2d_t normalmap2,
			 read_only image2d_t normalmap3,
			 read_only image2d_t normalmap4,
			 read_only image2d_t specularmap1,
			 read_only image2d_t specularmap2,
			 read_only image2d_t specularmap3,
			 read_only image2d_t specularmap4,
			 read_only image2d_t shadowmap,
			 unsigned int num_lights,
			 global struct Light *lights,
			 struct ViewInfo info)
{
	uint lx = get_local_id (0),
	    ly = get_local_id (1);
	uint gx = mul24 (get_group_id (0), get_local_size (0)),
	     gy = mul24 (get_group_id (1), get_local_size (1));

	uint x = gx + lx,
	    y = gy + ly;
	uint offset = mad24 (ly, get_local_size (0), lx);

	local ushort light_indices[256];
	float depths[4];
	float4 pos;
	local uint num_light_indices;
	num_light_indices = 0;

	local float4 boxmin, boxmax;

	boxmin.z = FLT_MAX;
	boxmax.z = FLT_MIN;

	pos.x = native_divide ((float)x,
	       		       (float)get_image_width (depthbuffer1));
     	pos.y = native_divide ((float)y,
	       		       (float)get_image_height (depthbuffer1));

     	depths[0] = read_imagef (depthbuffer1, sampler,
     	       	 	         (int2) (x, y)).x;
        depths[1] = read_imagef (depthbuffer2, sampler,
		     	       	 (int2) (x, y)).x;
	depths[2] = read_imagef (depthbuffer3, sampler,
		     	       	 (int2) (x, y)).x;
	depths[3] = read_imagef (depthbuffer4, sampler,
		     	       	 (int2) (x, y)).x;

	barrier (CLK_LOCAL_MEM_FENCE);

	// TODO: maybe atomic operations are in fact necessary
	for (int i = 0; i < 4; i++)
	{
		boxmin.z = min (boxmin.z, depths[i]);
		boxmax.z = max (boxmax.z, depths[i]);
	}
	
	switch (offset)
	{
		case 0:
		boxmin.x = native_divide ((float)gx,
			       	  	  (float)get_image_width
						 (depthbuffer1));
		break;
		case 1:
		boxmin.y = native_divide ((float)gy,
				       	  (float)get_image_width
						 (depthbuffer1));
		break;
		case 2:
		boxmax.x = native_divide ((float)(gx + get_local_size (0)),
			       	  	  (float)get_image_width
						 (depthbuffer1));
		break;
		case 3:
		boxmax.y = native_divide ((float)(gy + get_local_size (1)),
			       	  	  (float)get_image_width
						 (depthbuffer1));
		break;
	}

	barrier (CLK_LOCAL_MEM_FENCE);

	if (offset == 0)
	{
		float4 p;
		p = boxmin;
		getpos (&p, &info);
		boxmin = p;
	}
	else if (offset == 1)
	{
		float4 p;
		p = boxmax;
		getpos (&p, &info);
		boxmax = p;
	}

	barrier (CLK_LOCAL_MEM_FENCE);

	if (offset < num_lights)
	{

		// TODO: Actual light culling is needed here
		uint light = atom_inc (&num_light_indices);
		light_indices[light] = offset;
	}


	barrier (CLK_LOCAL_MEM_FENCE);

	float4 pixel4, pixel3, pixel2, pixel;
	pos.z = depths[0];
	pixel = compute_pixel (colormap1, pos, normalmap1,
      	       	 	       specularmap1, shadowmap, lx, ly,
		 	       x, y, lights, num_light_indices,
			       light_indices, &info);
	pos.z = depths[1];
	pixel2 = compute_pixel (colormap2, pos,
		       	 	normalmap2, specularmap2,
				shadowmap, lx, ly, x, y,
				lights, num_light_indices,
				light_indices, &info);
	pos.z = depths[2];
	pixel3 = compute_pixel (colormap3, pos,
	             	 	normalmap3, specularmap3,
				shadowmap, lx, ly, x, y,
				lights, num_light_indices,
				light_indices, &info);
	pos.z = depths[3];
	pixel4 = compute_pixel (colormap4, pos,
       	         	        normalmap4, specularmap4,
	        		shadowmap, lx, ly, x, y,
	        		lights, num_light_indices,
				light_indices, &info);
	pixel = mix (pixel4, pixel, pixel4.w);
	pixel = mix (pixel3, pixel, pixel3.w);
	pixel = mix (pixel2, pixel, pixel2.w);

	write_imagef (screen, (int2) (x, y), pixel);
}
