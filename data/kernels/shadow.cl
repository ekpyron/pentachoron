const sampler_t samplerA = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;
const sampler_t samplerB = CLK_NORMALIZED_COORDS_TRUE|CLK_FILTER_LINEAR
			  |CLK_ADDRESS_CLAMP_TO_EDGE;

struct ViewInfo
{
	float4 projinfo;
	float4 vmatinv[4];
	float4 shadowmat[4];
	float min_variance;
};

float3 getpos (uint x, uint y, read_only image2d_t depthbuffer,
               struct ViewInfo info)
{
	float depth = read_imagef (depthbuffer, samplerA, (int2) (x, y)).x;
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

	return pos.xyz;
}

float compute_shadow (int x, int y, read_only image2d_t depthbuffer,
      		      read_only image2d_t shadowmap, struct ViewInfo info)
{

	float4 pos;
	pos.xyz = getpos (x, y, depthbuffer, info);
	pos.w = 1.0;

	float4 lspos;

	lspos.x = dot (info.shadowmat[0], pos);
	lspos.y = dot (info.shadowmat[1], pos);
	lspos.z = dot (info.shadowmat[2], pos);
	lspos.w = dot (info.shadowmat[3], pos);
	lspos.xyz = native_divide (lspos.xyz, lspos.w);

	if (lspos.w < 0 || lspos.x < 0 || lspos.y < 0
	    || lspos.x > 1 || lspos.y > 1)
	{
	    return 0.0;
	}

	float2 moments;
	moments = read_imagef (shadowmap, samplerB, lspos.xy).xy;

	if (lspos.z <= moments.x)
	   return 1;

	float variance = moments.y - (moments.x * moments.x);
	variance = max (variance, info.min_variance);
	float d = lspos.z - moments.x;
	return variance / (variance + d * d);
}

kernel void genshadow (write_only image2d_t shadowmask,
       	    	       read_only image2d_t depthbuffer1,
       	    	       read_only image2d_t depthbuffer2,
       	    	       read_only image2d_t depthbuffer3,
       	    	       read_only image2d_t depthbuffer4,
	      	       read_only image2d_t shadowmap,
		       struct ViewInfo info)
{
	int x = get_global_id (0),
	    y = get_global_id (1);

	float4 shadow;

	shadow.x = compute_shadow (x, y, depthbuffer1, shadowmap, info);
	shadow.y = compute_shadow (x, y, depthbuffer2, shadowmap, info);
	shadow.z = compute_shadow (x, y, depthbuffer3, shadowmap, info);
	shadow.w = compute_shadow (x, y, depthbuffer4, shadowmap, info);

	write_imagef (shadowmask, (int2) (x, y), shadow);
}
