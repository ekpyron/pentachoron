#ifdef __CUDACC__
#pragma OPENCL EXTENSION cl_nv_pragma_unroll : enable
#endif


const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;
const sampler_t samplerB = CLK_NORMALIZED_COORDS_TRUE|CLK_FILTER_LINEAR
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

constant float4 small4 = (float4) (0.001, 0.001, 0.001, 0.001);

// maximum depth layers
#define DEPTHLAYERS 8
// maximum number of lights per tile
#define MAX_LIGHTS_PER_TILE 256

// the gbuffer data of one fragment.
struct PixelData
{
	float4 color;
	float4 specular;
	float4 normal;
	float depth;
};

// information about a light source.
struct Light
{
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
		union
		{
			struct
			{
				float4 left;
				float4 right;
				float4 bottom;
				float4 top;
				float4 near;
				float4 far;
			};
			float4 planes[6];
		};
	} frustum;
	struct
	{
		float scalar;
		float linear;
		float quadratic;
		float max_distance;
	} attenuation;
};

// general global information
struct Info
{
	float4 projinfo;
	float4 vmatinv[4];
	float4 shadowmat[4];
	float4 eye;
	float4 center;
	struct
	{	
		unsigned int size;
		float exponent;
		float threshold;
		float limit;
		
	} glow;
	struct
	{
		struct
		{
			float4 direction;
			float theta;
			float cos_theta;
			float padding[2];
		} sun;
		float turbidity;
		float perezY[5];
		float perezx[5];
		float perezy[5];
		float4 zenithYxy;
	} sky;
	float shadow_alpha;
	unsigned int mode;
	unsigned int num_lights;
	float screenlimit;
};

// material parameter
struct Parameter
{
	unsigned int model;
	union {
	      float smoothness;
	      float shininess;
	      float param1;
	};
	union {
	      float fresnel;
	      float gaussfactor;
	      float param2;
	};
	float padding;
};

// compute reflection vector
float3 reflect (float3 I, float3 N)
{
	return mad (-2.0 * dot (N, I), N, I);
}

// reconstruct position from depth
float getpos (float4 *pos, struct Info *info)
{
	float depth = pos->z;
	pos->w = 1;
	pos->xyz = mad (pos->xyz, 2, -1);

	float4 p;
	p.x = pos->x * info->projinfo.x;
	p.y = pos->y * info->projinfo.y;
	p.z = -pos->w;
	p.w = native_divide (pos->z * (info->projinfo.z - info->projinfo.w)
	      		     + pos->w * (info->projinfo.z + info->projinfo.w),
			     2 * info->projinfo.z * info->projinfo.w);

	if (depth == 1.0)
	{
		pos->x = dot ((float4) (info->vmatinv[0].xyz, 0), p);
		pos->y = dot ((float4) (info->vmatinv[1].xyz, 0), p);
		pos->z = dot ((float4) (info->vmatinv[2].xyz, 0), p);
		pos->w = 1.0;
		return depth;
	}

	pos->x = dot (info->vmatinv[0], p);
	pos->y = dot (info->vmatinv[1], p);
	pos->z = dot (info->vmatinv[2], p);
	pos->w = dot (info->vmatinv[3], p);
	pos->xyz = native_divide (pos->xyz, pos->w);
	pos->w = 1.0;

	return depth;
}

float perez (float cos_theta, float gamma, float cos_gamma,
      	     float c[5])
{
	return (1 + c[0] * exp (c[1] / cos_theta))
	       * (1 + c[2] * exp (c[3] * gamma)
	       	  + c[4] * cos_gamma * cos_gamma);
}

float3 Yxy2RGB (float3 Yxy)
{
	float3 XYZ;
	float3 rgb;
	XYZ.x = Yxy.x * Yxy.y / Yxy.z;
	XYZ.y = Yxy.x;
	XYZ.z = Yxy.x * (1 - Yxy.y - Yxy.z) / Yxy.z;

	rgb.x = dot ((float3) (2.3706743f, -0.9000405f,-0.4706338f), XYZ);
	rgb.y = dot ((float3) (-0.5138850f, 1.4253036f, 0.0885814f), XYZ);
	rgb.z = dot ((float3) (0.0052982f, -0.0146949f, 1.0093968f), XYZ);

	return rgb;
}

// computes sky color (TODO)
float4 compute_sky (float4 p, struct Info *info)
{
	float T = info->sky.turbidity;

	float theta_s = info->sky.sun.theta;
	float cos_theta_s = info->sky.sun.cos_theta;

	float3 dir;
	float3 sundir;

	sundir = info->sky.sun.direction.xyz;
	dir = fast_normalize (p.xyz);

	float cos_theta = dir.y;
	float theta = acos (cos_theta);
	if (cos_theta < 0)
	{
	   //return (float4) (0, 0, 0, 1);
	   cos_theta = -cos_theta;
	}

	float cos_gamma = dot (sundir, dir);
	float gamma = acos (cos_gamma);

	float3 Yxy;

	Yxy.x = info->sky.zenithYxy.x
	      	* perez (cos_theta, gamma, cos_gamma,
	      	         info->sky.perezY)
		/ perez (1, theta_s, cos_theta_s,
		    	 info->sky.perezY);

	Yxy.y = info->sky.zenithYxy.y
	      	* perez (cos_theta, gamma, cos_gamma,
	      	       	 info->sky.perezx)
		/ perez (1, theta_s, cos_theta_s,
		    	 info->sky.perezx);

	Yxy.z = info->sky.zenithYxy.z
	      	* perez (cos_theta, gamma, cos_gamma,
	      	       	 info->sky.perezy)
		/ perez (1, theta_s, cos_theta_s,
		    	 info->sky.perezy);

	float4 color = (float4) (0.04*Yxy2RGB (Yxy), 1.0);
	return clamp (color, 0.0, 2.0);
}

float specular_gaussian (float3 normal, float3 halfVec,
      			 struct Parameter *param)
{
	float e;
	e = acos (dot (normal, halfVec));
	e = native_divide (clamp (e, 0.0, 1.0), param->smoothness);
	return param->gaussfactor * native_exp (-e * e);
}

float specular_phong (float3 viewDir, float3 lightDir, float3 normal,
      		      struct Parameter *param)
{
	float k;
	k = dot (viewDir, reflect (-lightDir, normal));
	k = native_powr (k, param->shininess);
	return k;
}

float specular_beckmann (float3 normal, float3 halfVec,
      			 struct Parameter *param)
{
	float e, m;
	e = acos (dot (normal, halfVec));
	e = clamp (e, 0.0, 1.0);
	e = native_cos (e);
	e = e * e;
	m = param->smoothness;
	m = m * m;
	return native_divide (native_exp (-native_divide (1 - e, e * m)),
	       		      (M_PI * m * e * e));
}

float specular_cooktorrance (float3 viewDir, float3 lightDir,
      			     float3 normal, float3 halfVec,
      			     struct Parameter *param)
{
	float fresnel = param->fresnel;
	float k;

	k = specular_beckmann (normal, halfVec, param);

	float NdotH, VdotH, NdotV, NdotL;
	NdotH = dot (halfVec, normal);
	VdotH = dot (viewDir, halfVec);
	NdotV = dot (normal, viewDir);
	NdotL = dot (normal, lightDir);

	float f;
	f = fresnel + pown (1 - NdotV, 5) * (1 - fresnel);
	float g, g1, g2;
	g1 = native_divide (2 * NdotH * NdotV, VdotH);
	g2 = native_divide (2 * NdotH * NdotL, VdotH);
	g = min (1.0, min (g1, g2));
	return native_divide (k * f * g, NdotV);
}

float compute_shadow (read_only image2d_t shadowmap, float4 pos,
		      struct Info *info)
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
	float2 moments = read_imagef (shadowmap, samplerB,
	       	       	 	      lspos.xy).xy;

	if (lspos.z <= moments.x)
	{
	   return 1.0;
	}

	float variance = moments.y - (moments.x * moments.x);
	variance = max (variance, 0.00001);
	float d = lspos.z - moments.x;
	float p = native_divide (variance, variance + d * d);
	return smoothstep (0.1, 1.0, p);
}

// compute the pixel value for some given gbuffer data
float4 compute_pixel (struct PixelData *data, float2 p,
       		      global struct Light *lights,
		      unsigned int num_light_indices,
		      local ushort *light_indices,
		      read_only image2d_t shadowmap,
		      struct Info *info,
		      unsigned int num_parameters,
		      global struct Parameter *parameters,
		      bool *issky)
{
	float4 pos;
	uint offset = mad24 (get_local_id (1), get_local_size (0),
	     	      	     get_local_id (0));
	uint x = get_global_id (0), y = get_global_id (1);
	float3 diffuse = (float3) (0, 0, 0);
	float3 specular = (float3) (0, 0, 0);
	uint material;
	struct Parameter param;

	pos.xy = p;
	pos.z = data->depth;
	pos.w = 1.0;

	// reconstruct world space position
	if (getpos (&pos, info) == 1.0)
	{
	   *issky = true;
	   return compute_sky (pos, info);
	}

	*issky = false;

	float3 viewDir = fast_normalize
	       	       	 (info->eye.xyz - pos.xyz);

	// get material information
	material = data->specular.w * 255.0;
	if (material >= num_parameters)
	{
		return (float4) (1.0, 0.0, 0.0, 1.0);
	}
	param = parameters[material];

	// iterate over lights
	for (int i = 0; i < min ((uint)MAX_LIGHTS_PER_TILE,
	    	     	    	 num_light_indices); i++)
	{
		global struct Light *light;

		// get per-light information
		light = &lights[light_indices[i]];

		// compute distance and direction of the light
		float3 lightDir = light->position.xyz - pos.xyz;
		float dist = fast_length (lightDir);
		lightDir = native_divide (lightDir, dist);

		// compute the distance attenuation
		float attenuation;
		attenuation = native_recip
			    (mad (light->attenuation.quadratic, dist * dist,
			    	  mad (light->attenuation.linear,
			    	       dist, light->attenuation.scalar)));
		if (attenuation < 0.001)
		   continue;

		float angle;
		
		// compute the (cosine of the) angle to spot light direction
		angle = dot (fast_normalize (light->direction.xyz),
			     -lightDir);

		// cull everything outside the spot angle
		if (angle < light->spot.cosine)
		   continue;

		// apply penumbra
		if (angle < light->spot.penumbra.cosine)
		{
		   attenuation *= native_divide (angle - light->spot.cosine,
		   			         light->spot.penumbra.cosine
				   	         - light->spot.cosine);
		}

		// apply spot exponent
		attenuation *= native_powr (angle, light->spot.exponent);

		if (attenuation < 0.001)
		   continue;
		
		// fetch normal
		float3 normal = mad (data->normal.xyz, 2, -1);

		// compute normal dot light direction
		float NdotL;
		NdotL = max (dot (normal, lightDir), 0.0f);
		float3 halfVec = fast_normalize (viewDir + lightDir);
		float d = 0.0, s = 0.0;

		switch (param.model)
		{
			case 0:
			     d = NdotL;
			     break;
			case 1:
			     d = NdotL;
			     s = specular_gaussian (normal, halfVec, &param);
			     break;
			case 2:
			     d = NdotL;
			     s = specular_phong (viewDir, lightDir,
			       	 		 normal, &param);
			     break;
			case 3:
			     d = NdotL;
			     s = specular_beckmann (normal, halfVec,
				       	 	    &param);
			     break;
			case 4:
			     d = NdotL;
			     s = specular_cooktorrance (viewDir, lightDir,
			       	 		        normal, halfVec,
							&param);
			     break;
			default:
			     break;
		}
		// add this light to overall diffuse light color
		diffuse += attenuation * light->color.xyz * d;
		specular += attenuation * light->specular.xyz * s;
	}

	float4 pixel;
	if (info->mode == 2) // no color; only lighting
	{
		pixel = (float4) (1, 1, 1, data->color.w);
		pixel.xyz *= diffuse;
	}
	else // normal operation
	{
		// fetch color
		pixel = data->color;
		
		// apply diffuse light
		pixel.xyz *= diffuse;

		// apply specular light, if any
		if (any (specular > small4))
		{
			specular *= data->specular.xyz;
			pixel.xyz += specular;
		}
	}

	// clamp the pixel
	pixel = clamp (pixel, 0.0, info->screenlimit);

	float shadow = compute_shadow (shadowmap, pos, info);
	pixel *= mad (shadow, info->shadow_alpha, 1 - info->shadow_alpha);

	return pixel;
}

// determine whether a light affects a given bounding box
// TODO: fix the box culling (commented out below;
//   	 it produces artifacts for specific angles)
bool culllight (global struct Light *light, float3 boxmin, float3 boxmax)
{
	float3 sphere;
	float radius;
	radius = 0.5 * fast_distance (boxmin, boxmax);
	sphere = 0.5 * (boxmin + boxmax);

	float dist;
	#pragma unroll
	for (uchar i = 0; i < 6; i++)
	{
		dist = dot (light->frustum.planes[i].xyz, sphere)
		       + light->frustum.planes[i].w;
		if (dist <= -radius)
		   return false;
	}
	return true;
	
/*	#pragma unroll
	for (uchar i = 0; i < 6; i++)
	{
		float4 plane = light->frustum.planes[i];
		if (dot (plane.xyz, boxmin) + plane.w > 0)
		    continue;
		if (plane.x * boxmax.x + plane.y * boxmin.y
		    + plane.z * boxmin.z + plane.w > 0)
		    continue;
		if (plane.x * boxmin.x + plane.y * boxmax.y
		    + plane.z * boxmin.z + plane.w > 0)
		    continue;
		if (plane.x * boxmax.x + plane.y * boxmax.y
		    + plane.z * boxmin.z + plane.w > 0)
		    continue;
		if (plane.x * boxmin.x + plane.y * boxmin.y
		    + plane.z * boxmax.z + plane.w > 0)
		    continue;
		if (plane.x * boxmax.x + plane.y * boxmin.y
		    + plane.z * boxmax.z + plane.w > 0)
		    continue;
		if (plane.x * boxmin.x + plane.y * boxmax.y
		    + plane.z * boxmax.z + plane.w > 0)
		    continue;
		if (dot (plane.xyz, boxmax) + plane.w > 0)
		    continue;
		return false;
	}

	return true;*/
}

// determine which lights affect the current tile
void getlightindices (local ushort *light_indices,
     		      local uint *num_light_indices,
     		      int boxmin_int, int boxmax_int,
		      global struct Light *lights, uint num_lights,
		      float3 boxmin, float3 boxmax)
{
	uint offset = mad24 (get_local_id (1), get_local_size (0),
	     	      	     get_local_id (0));

	if (boxmin_int == 4294967295 && boxmax_int == 0)
	   return;

	// iterate over every light
	// TODO: this still assumes hard-coded 256 threads
	for (int pass = 0; pass < ((num_lights+255)>>8); pass++)
	{
		if (offset + (pass<<8) < num_lights)
		{
			// perform the light culling
			if (culllight (&lights[(pass<<8) + offset],
			   	       boxmin, boxmax))
			{
				// add the light to the per work-group
				// (i.e. per tile) list of lights
				uint index = atom_inc (num_light_indices);
				if (index < MAX_LIGHTS_PER_TILE)
			   	   light_indices[index] = (pass<<8) + offset;
				else
				   return;
			}
		}
	}
}

// unpacks an integer value to a vector of floats
float4 unpackUnorm4x8 (uint val)
{
	float4 retval;
	uchar4 ints;
	ints = vload4 (0, (uchar*) &val);
	retval = convert_float4 (ints);
	return native_divide (retval, 255);
}

// the composition kernel
// computes the (lit) color value (and the glow map value) from the gbuffer
// for every screen pixel
kernel void composition (write_only image2d_t screen,
       	    		 write_only image2d_t glowmap,
	      		 read_only image2d_t colormap,
			 read_only image2d_t depthbuffer,
			 read_only image2d_t normalmap,
			 read_only image2d_t specularmap,
			 read_only image2d_t shadowmap,
			 read_only image2d_t fragidx,
			 global uint *fraglist,
			 global struct Light *lights,
			 struct Info info,
			 unsigned int num_parameters,
			 global struct Parameter *parameters)
{
	uint lx = get_local_id (0),
	    ly = get_local_id (1);
	uint offset = mad24 (ly, get_local_size (0), lx);
	uint gx = mul24 (get_group_id (0), get_local_size (0)),
	     gy = mul24 (get_group_id (1), get_local_size (1));

	uint x = get_global_id (0),
	    y = get_global_id (1);

	local float gxf, gyf;
	local uint boxmin_int, boxmax_int;
	local uint num_light_indices;

	struct PixelData data[DEPTHLAYERS];
	uchar num;

	// precompute some values needed later
   	gxf = native_divide ((float)gx, 
      	       		     (float)get_image_width (depthbuffer));
   	gyf = native_divide ((float)gy, 
      	 	             (float)get_image_width (depthbuffer));
	num_light_indices = 0;
	boxmin_int = 4294967295;
	boxmax_int = 0;

	local ushort light_indices[MAX_LIGHTS_PER_TILE];
	uint indices[DEPTHLAYERS];
	float depth;
	float2 pos;

	// compute position from global ids
	pos.x = native_divide ((float)x,
	       		       (float)get_image_width (depthbuffer));
     	pos.y = native_divide ((float)y,
	       		       (float)get_image_height (depthbuffer));

	// obtain the starting index of the current pixel in the
	// fragment list (for order independent transparency)
	uint idx = read_imageui (fragidx, sampler, (int2) (x, y)).x;

	num = 0;
	// store the first up to DEPTHLAYERS fragments from the list
	while (idx != (uint) -1)
	{
		// fetch depth
		float d = as_float (fraglist[5 * idx + 3]);

		// insertion of the current fragment at the right place

		// still place in the array?
		if (num < DEPTHLAYERS)
		{
			num++;
		}
		// if not check whether to override the last entry
		else if (as_float (fraglist[5 * indices[num - 1] + 3]) <= d)
		{
			// discard the current fragment and
			// continue with the next one
			idx = fraglist[5 * idx + 4];
			continue;
		}

		// move the indices in the list to the right
		// as long as the one to the left has a smaller depth
		// than the fragment we're processing
		uint i = num - 1;
		while (i > 0 && d < as_float (fraglist[5 * indices[i - 1] + 3]))
		{
			indices[i] = indices[i - 1];
			i--;
		}
		// store the index of this fragment in the resulting hole
		// in the list
		indices[i] = idx;

		// fetch next index
		idx = fraglist[5 * idx + 4];
	}
	// load the gbuffer data for every fragment in the list
	for (uchar i = 0; i < num; i++)
	{
		idx = indices[i];
		data[i].color = unpackUnorm4x8 (fraglist[5 * idx + 0]);
		data[i].specular = unpackUnorm4x8 (fraglist[5 * idx + 1]);
		data[i].normal = unpackUnorm4x8 (fraglist[5 * idx + 2]);
		data[i].depth = as_float (fraglist[5 * idx + 3]);
	}

	barrier (CLK_LOCAL_MEM_FENCE);

	// compute the minimum and maximum depth per tile

	// fetch opaque depth
     	depth = read_imagef (depthbuffer, sampler,
     	       	 	     (int2) (x, y)).x;
	// compute minimum/maximum
	if (depth < 1.0)
	{
		uint d = (uint) (depth * 4294967295.0);
		atomic_min (&boxmin_int, d);
		atomic_max (&boxmax_int, d);
	}

	// do the same for every transparent fragment in our list
	for (uchar i = 0; i < num; i++)
	{
		if (data[i].depth < 1.0)
		{
			uint d = (uint) (data[i].depth * 4294967295.0);
			atomic_min (&boxmin_int, d);
			atomic_max (&boxmax_int, d);
		}
	}

	barrier (CLK_LOCAL_MEM_FENCE);

	// compute bounding box
	float4 boxmin, boxmax;
	boxmin.x = gxf;
	boxmin.y = gyf;
	boxmin.z = native_divide ((float)boxmin_int, 4294967295.0);
	getpos (&boxmin, &info);
	boxmax.x = gxf;
	boxmax.y = gyf;
	boxmax.z = native_divide ((float)boxmax_int, 4294967295.0);
	getpos (&boxmax, &info);

	// obtain the light indices affecting the current tile
	getlightindices (light_indices, &num_light_indices, boxmin_int,
			 boxmax_int, lights, info.num_lights,
			 boxmin.xyz, boxmax.xyz);

	barrier (CLK_LOCAL_MEM_FENCE);

	float4 pixel;

	if (info.mode == 1) { // output number of lighs per tile
	
	float f = native_divide ((float) num_light_indices,
	      	  		 MAX_LIGHTS_PER_TILE - 1);

	if (f < 0.25)
	   pixel = 2 * f * ((float4) (0, 0, 1, 1));
	else if (f < 0.5)
	   pixel = (0.5 + 2 * (f - 0.25)) * ((float4) (0, 1, 0, 1));
	else if (f < 0.75)
	   pixel = (0.5 + 2 * (f - 0.5)) * ((float4) (1, 1, 0, 1));
	else if (f < 1)
	   pixel = (0.5 + 2 * (f - 0.75)) * ((float4) (1, 0, 0, 1));
	else
	   pixel = (float4) (1, 1, 1, 1);
	write_imagef (screen, (int2) (x, y), pixel);
	write_imagef (glowmap, (int2) (x, y), (float4) (0, 0, 0, 0));

	} else { // normal operation

	// fetch the gbuffer data for the opaque component
	struct PixelData opaquedata;
	opaquedata.color = read_imagef (colormap, sampler, (int2) (x, y));
	opaquedata.specular = read_imagef (specularmap, sampler,
			      		   (int2) (x, y));
	opaquedata.normal = read_imagef (normalmap, sampler, (int2) (x, y));
	opaquedata.depth = depth;

	// TODO: this variable is an evil hack
	bool issky;

	// compute the opaque color
	pixel = compute_pixel (&opaquedata, pos, lights, num_light_indices,
	      		       light_indices, shadowmap, &info,
			       num_parameters, parameters, &issky);

	// iterate over the depth layers
	for (uchar i = 0; i < num; i++)
	{
		float4 pixel2;
		bool ignored; // TODO: evil hack continued...

		// compute the color for a depth layer
		// (back to front)
		pixel2 = compute_pixel (&data[num - i - 1], pos, lights,
		       	 	        num_light_indices, light_indices,
		       	 	        shadowmap, &info,
					num_parameters, parameters, &ignored);
		// blend the next layer with the current colot
		pixel = mix (pixel, pixel2, pixel2.w);
	}

	// write the screen value
	write_imagef (screen, (int2) (x, y), pixel);

	// glow effect
	if (info.glow.size > 0)
	{
		if (issky)
		{
			write_imagef (glowmap, (int2) (x, y),
				      (float4) (0.0, 0.0, 0.0, 0.0));
			return;
		}

		pixel = native_powr (pixel, info.glow.exponent);
		// compute the luminance of the current pixel
		float luminance = dot ((float3) (0.2126, 0.7152, 0.0722),
		      		       pixel.xyz);

		float4 glow = (float4) (0.0, 0.0, 0.0, 0.0);

		// check against the luminance threshold
		if (luminance > info.glow.threshold)
		{
			glow.xyz = pixel.xyz;
			glow.w = luminance;
		}

		glow = clamp (glow, 0.0, info.glow.limit);

		// write to glow map
		write_imagef (glowmap, (int2) (x, y), glow);
	}

	} /* info.mode */
}
