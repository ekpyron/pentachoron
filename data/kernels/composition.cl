const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;
const sampler_t samplerB = CLK_NORMALIZED_COORDS_TRUE|CLK_FILTER_LINEAR
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

constant float4 small4 = (float4) (0.001, 0.001, 0.001, 0.001);

struct PixelData
{
	float4 color;
	float4 specular;
	float4 normal;
	float depth;
};

struct Light
{
	union {
	      struct {
	      	     float4 position;
		     float4 color;
		     float4 direction;
		     struct
		     {
			float cosine;
			float exponent;
			float angle;
			float tangens;
			struct
			{
				float angle;
				float cosine;
				float padding[2];
			} penumbra;
		     } spot;
		     float4 specular;
		     struct
		     {
			float scalar;
			float linear;
			float quadratic;
			float max_distance;
		     } attenuation;
	      };
	      float4 data[6];
	};
};

struct Info
{
	float4 projinfo;
	float4 vmatinv[4];
	float4 shadowmat[4];
	float4 eye;
	float4 center;
	float luminance_threshold;
	float shadow_alpha;
	unsigned int glowsize;
	float padding;
};

struct Parameter
{
	/*
	 * specular.x: specular exponent
	 */
	struct
	{
		float exponent;
		float padding[3];
	} specular;
};

float3 reflect (float3 I, float3 N)
{
	return mad (-2.0 * dot (N, I), N, I);
}

float getpos (float4 *pos, struct Info *info)
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
      		      read_only image2d_t shadowmap, struct Info *info)
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
	variance = max (variance, 0.00001);
	float d = lspos.z - moments.x;
	float p = native_divide (variance, variance + d * d);
	return smoothstep (0.1, 1.0, p);
}

float4 compute_sky (float4 p, struct Info *info)
{
	float4 color;
	color.xyz = (float3) (0.0, 0.0, 0.10);
	color.w = 1.0;

	return color;
}

float4 unpackUnorm4x8 (uint val)
{
	float4 retval;
	uchar4 ints;
	ints = vload4 (0, (uchar*) &val);
	retval = convert_float4 (ints);
	return native_divide (retval, 255);
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
		      struct Info *info,
		      unsigned int num_parameters,
		      global struct Parameter *parameters)
{
	float4 pos = p;
	if (getpos (&pos, info) == 1.0)
	   return compute_sky (p, info);
	float3 diffuse = (float3) (0, 0, 0);
	float3 specular = (float3) (0, 0, 0);
	uint offset = mad24 (ly, get_local_size (0), lx);
	uint material;
	float4 speculartmp = read_imagef (specularmap, sampler,
			    		  (int2) (x, y));
	material = speculartmp.w * 255.0;

	if (material >= num_parameters)
	{
		return (float4) (1.0, 0.0, 0.0, 1.0);
	}

	struct Parameter param;

	param = parameters[material];

	for (int i = 0; i < num_light_indices; i++)
	{
		struct Light light;
		light = lights[light_indices[i]];
		float3 lightDir = light.position.xyz - pos.xyz;

		float dist = fast_length (lightDir);
		lightDir = native_divide (lightDir, dist);

		float attenuation;
		attenuation = native_recip
			    (mad (light.attenuation.quadratic, dist * dist,
			    	  mad (light.attenuation.linear,
			    	       dist, light.attenuation.scalar)));
		if (attenuation < 0.001)
		   continue;

		float spotEffect = 1.0;
		float angle;
		
		angle = dot (fast_normalize (light.direction.xyz),
			     -lightDir);
		if (angle < light.spot.cosine)
		   continue;
		if (angle < light.spot.penumbra.cosine)
		{
		   spotEffect = (angle - light.spot.cosine)
		   		/ (light.spot.penumbra.cosine
				   - light.spot.cosine);
		}

		spotEffect *= native_powr (angle, light.spot.exponent);

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

		if (param.specular.exponent != 0.0)
		    specular += attenuation * light.specular.xyz
				* native_powr (r, param.specular.exponent);
	}

	float shadow;
	shadow = compute_shadow (x, y, pos, shadowmap, info);
	shadow *= info->shadow_alpha;
	shadow += 1 - info->shadow_alpha;
	diffuse *= shadow;
	specular *= shadow;

	diffuse += 0.025;

	float4 pixel = read_imagef (colormap, sampler, (int2) (x, y));

	pixel.xyz *= diffuse;

	if (any (specular > small4))
	{
		specular *= speculartmp.xyz;
		pixel.xyz += specular;
	}

	return pixel;
}

void sortByDepth (struct PixelData *data, uchar num, uchar indices[8])
{
	uchar i;
	for (i = 0; i < num; i++)
	{
		indices[i] = i;
	}
	do
	{
		for (i = 1; i < num; i++)
		{
			if (data[indices[i - 1]].depth
			    > data[indices[i]].depth)
			{
				uchar tmp;
				tmp = indices[i];
				indices[i] = indices[i - 1];
				indices[i - 1] = tmp;
				break;
			}
		}
	} while (i < num);
}

kernel void composition (write_only image2d_t screen,
       	    		 write_only image2d_t glowmap,
	      		 read_only image2d_t colormap,
			 read_only image2d_t depthbuffer,
			 read_only image2d_t normalmap,
			 read_only image2d_t specularmap,
			 read_only image2d_t shadowmap,
			 read_only image2d_t fragidx,
			 global uint *fraglist,
			 unsigned int num_lights,
			 global struct Light *lights,
			 struct Info info,
			 unsigned int num_parameters,
			 global struct Parameter *parameters)
{
	uint lx = get_local_id (0),
	    ly = get_local_id (1);
	uint gx = mul24 (get_group_id (0), get_local_size (0)),
	     gy = mul24 (get_group_id (1), get_local_size (1));

	uint x = gx + lx,
	    y = gy + ly;
	uint offset = mad24 (ly, get_local_size (0), lx);


	local float gxf, gyf;
	local float4 sphere;
	local float radius;
	local uint boxmin_int, boxmax_int;
	local uint num_light_indices;

	switch (offset)
	{
	   case 0:
	   	gxf = native_divide ((float)gx, 
	       	       		     (float)get_image_width (depthbuffer));
	   break;
	   case 1:
	   	gyf = native_divide ((float)gy, 
	       	 	             (float)get_image_width (depthbuffer));
	   break;
	   case 2:
		num_light_indices = 0;
		boxmin_int = 4294967295;
		boxmax_int = 0;
	   break;
	}

	local ushort light_indices[256];
	float depth;
	float4 pos;

	pos.x = native_divide ((float)x,
	       		       (float)get_image_width (depthbuffer));
     	pos.y = native_divide ((float)y,
	       		       (float)get_image_height (depthbuffer));

	barrier (CLK_LOCAL_MEM_FENCE);

     	depth = read_imagef (depthbuffer, sampler,
     	       	 	     (int2) (x, y)).x;

	if (depth < 1.0)
	{
		uint d = (uint) (depth * 4294967295.0);
		atomic_min (&boxmin_int, d);
		atomic_max (&boxmax_int, d);
	}	

	barrier (CLK_LOCAL_MEM_FENCE);

	if (offset == 0)
	{
		float4 boxmin, boxmax;
		boxmin.x = gxf;
		boxmin.y = gyf;
		boxmin.z = native_divide ((float)boxmin_int, 4294967295.0);
		getpos (&boxmin, &info);
		boxmax.x = gxf;
		boxmax.y = gyf;
		boxmax.z = native_divide ((float)boxmax_int, 4294967295.0);
		getpos (&boxmax, &info);
		radius = 0.5 * fast_distance (boxmin, boxmax);
		sphere = 0.5 * (boxmin + boxmax);
	}

	barrier (CLK_LOCAL_MEM_FENCE);

	if (boxmin_int != 4294967295 || boxmax_int != 0) {

	for (int pass = 0; pass < ((num_lights+255)>>8); pass++)
	{

	if (offset < num_lights)
	{
		global struct Light *light;

		float lambda;
		light = &lights[(pass<<8) + offset];
		lambda = native_divide (dot (light->direction,
		       	 	       	     sphere - light->position),
					dot (light->direction,
					    light->direction));

		if (lambda >= 0 && lambda - radius
		    < light->attenuation.max_distance)
		{

			float r = mad (light->spot.tangens, lambda,
			      	       radius);

			float d = fast_distance (mad (lambda, light->direction,
			      	  		      light->position),
						 sphere);

			if (d <= r)
			{
				uint index = atom_inc (&num_light_indices);
				if (index < 256)
				   light_indices[index] = (pass<<8) + offset;
				else
				   num_light_indices = 255;
			}
		}
	}

	}
	}


	barrier (CLK_LOCAL_MEM_FENCE);

	float4 pixel4, pixel3, pixel2, pixel;

/*	float f = ((float) num_light_indices) / 32.0f;
	pixel = f * ((float4) (1, 1, 1, 1));*/

	uint idx = read_imageui (fragidx, sampler, (int2) (x, y)).x;

	pos.z = depth;
	pixel = compute_pixel (colormap, pos, normalmap,
      	       	 	       specularmap, shadowmap, lx, ly,
		 	       x, y, lights, num_light_indices,
			       light_indices, &info,
			       num_parameters, parameters);

	if (idx != (uint) -1)
	{
		struct PixelData data[8];
		uchar indices[8];
		uchar num;
		num = 0;
		while (idx != (uint) -1)
		{
			data[num].color = unpackUnorm4x8
					  (fraglist[5 * idx + 0]);
			data[num].specular = unpackUnorm4x8
					     (fraglist[5 * idx + 1]);
			data[num].normal = unpackUnorm4x8
					   (fraglist[5 * idx + 2]);
			data[num].depth = as_float (fraglist[5 * idx + 3]);
			num++;
			idx = fraglist[5 * idx + 4];
		}
		sortByDepth (data, num, indices);
		for (uchar i = 1; i <= num; i++)
		{
			pixel = mix (pixel, data[indices[num - i]].color,
			      	     data[indices[num - i]].color.w);
//			pixel = (float4) (1.0, 0.0, 0.0, 1.0);
//			pixel = data[indices[num - 1]].color;
		}
	}


	float luminance = 0.2126 * pixel.x + 0.7152 * pixel.y
	      		  + 0.0722 * pixel.w;

	if (info.glowsize > 0)
	{
		float4 glow = (float4) (0.0, 0.0, 0.0, 0.0);

		if (luminance > info.luminance_threshold)
		{
			glow.xyz = pixel.xyz;
			glow.w = luminance;
		}

		write_imagef (glowmap, (int2) (x, y), glow);
	}

	write_imagef (screen, (int2) (x, y), pixel);
}
