const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE|CLK_FILTER_NEAREST
			  |CLK_ADDRESS_CLAMP_TO_EDGE;
__constant float COS_8 = 0.923879532511f;
__constant float SIN_8 = 0.382683432365f;
constant float PI = 3.141592653589f;

#define MUL_RE(a,b) (a.even*b.even - a.odd*b.odd)
#define MUL_IM(a,b) (a.even*b.odd + a.odd*b.even)

float2 mul_1(float2 a,float2 b)
{
	float2 x;
	x.even = MUL_RE(a,b);
	x.odd = MUL_IM(a,b);
	return x;
}

// mul_p*q*(a) returns a*EXP(-I*PI*P/Q)
#define mul_p0q1(a) (a)

#define mul_p0q2 mul_p0q1
float2  mul_p1q2(float2 a) { return (float2)(a.y,-a.x); }

__constant float SQRT_1_2 = 0.707106781188f; // cos(PI/4)
#define mul_p0q4 mul_p0q2
float2  mul_p1q4(float2 a) { return (float2)(SQRT_1_2)*(float2)(a.x+a.y,-a.x+a.y); }
#define mul_p2q4 mul_p1q2
float2  mul_p3q4(float2 a) { return (float2)(SQRT_1_2)*(float2)(-a.x+a.y,-a.x-a.y); }

#define mul_p0q8 mul_p0q4
float2  mul_p1q8(float2 a) { return mul_1((float2)(COS_8,-SIN_8),a); }
#define mul_p2q8 mul_p1q4
float2  mul_p3q8(float2 a) { return mul_1((float2)(SIN_8,-COS_8),a); }
#define mul_p4q8 mul_p2q4
float2  mul_p5q8(float2 a) { return mul_1((float2)(-SIN_8,-COS_8),a); }
#define mul_p6q8 mul_p3q4
float2  mul_p7q8(float2 a) { return mul_1((float2)(-COS_8,-SIN_8),a); }

#define DFT2_TWIDDLE(a,b,t) { float2 tmp = t(a-b); a += b; b = tmp; }

float2 exp_alpha (float alpha)
{
	float cs, sn;
	sn = sincos (alpha, &cs);
	return (float2) (cs, sn);
}

kernel void fft (read_only image2d_t in, global float2 *out)
{
	int width = get_image_width (in);

	int y = get_global_id (1);

	float2 u[16];

	int offset = get_global_id (0);

	int x = offset << 4;
	for (int m = 0; m < 16; m++)
	{
		u[m].x = read_imagef (in, sampler, (int2)(x + m, y)).x;
		u[m].y = 0;
	}

	float alpha = -PI * (float) (offset) / (float) (8 * width);
	for (int m = 1; m < 16; m++)
	{
		u[m] = mul_1 (exp_alpha (m * alpha), u[m]);
	}

	DFT2_TWIDDLE(u[0],u[8],mul_p0q8);	
	DFT2_TWIDDLE(u[1],u[9],mul_p1q8);
	DFT2_TWIDDLE(u[2],u[10],mul_p2q8);
	DFT2_TWIDDLE(u[3],u[11],mul_p3q8);
	DFT2_TWIDDLE(u[4],u[12],mul_p4q8);
	DFT2_TWIDDLE(u[5],u[13],mul_p5q8);
	DFT2_TWIDDLE(u[6],u[14],mul_p6q8);
	DFT2_TWIDDLE(u[7],u[15],mul_p7q8);

	DFT2_TWIDDLE(u[0],u[4],mul_p0q4);
  	DFT2_TWIDDLE(u[1],u[5],mul_p1q4);
	DFT2_TWIDDLE(u[2],u[6],mul_p2q4);
 	DFT2_TWIDDLE(u[3],u[7],mul_p3q4);
  	DFT2_TWIDDLE(u[8],u[12],mul_p0q4);
  	DFT2_TWIDDLE(u[9],u[13],mul_p1q4);
  	DFT2_TWIDDLE(u[10],u[14],mul_p2q4);
  	DFT2_TWIDDLE(u[11],u[15],mul_p3q4);

  	DFT2_TWIDDLE(u[0],u[2],mul_p0q2);
  	DFT2_TWIDDLE(u[1],u[3],mul_p1q2);
  	DFT2_TWIDDLE(u[4],u[6],mul_p0q2);
  	DFT2_TWIDDLE(u[5],u[7],mul_p1q2);
  	DFT2_TWIDDLE(u[8],u[10],mul_p0q2);
  	DFT2_TWIDDLE(u[9],u[11],mul_p1q2);
  	DFT2_TWIDDLE(u[12],u[14],mul_p0q2);
  	DFT2_TWIDDLE(u[13],u[15],mul_p1q2);

	out[y * width + x + 0] = u[0] + u[1];
	out[y * width + x + 1] = u[8] + u[9];
	out[y * width + x + 2] = u[4] + u[5];
	out[y * width + x + 3] = u[12] + u[13];
	out[y * width + x + 4] = u[2] + u[3];
	out[y * width + x + 5] = u[10] + u[11];
	out[y * width + x + 6] = u[6] + u[7];
	out[y * width + x + 7] = u[14] + u[15];
	out[y * width + x + 8] = u[0] - u[1];
	out[y * width + x + 9] = u[8] - u[9];
	out[y * width + x + 10] = u[4] - u[5];
	out[y * width + x + 11] = u[12] - u[13];
	out[y * width + x + 12] = u[2] - u[3];
	out[y * width + x + 13] = u[10] - u[11];
	out[y * width + x + 14] = u[6] - u[7];
	out[y * width + x + 15] = u[14] - u[15];
}

kernel void ifft (global float2 *in, write_only image2d_t out)
{
	int width = get_image_width (out);

	int y = get_global_id (1);

	float2 u[16];

	int offset = get_global_id (0);

	int x = offset << 4;
	for (int m = 0; m < 16; m++)
	{
		u[m].x = in[y * width + x + m].x;
		u[m].y = -in[y * width + x + m].y;
	}

	float alpha = -PI * (float) (offset) / (float) (8 * width);
	for (int m = 1; m < 16; m++)
	{
		u[m] = mul_1 (exp_alpha (m * alpha), u[m]);
	}

	DFT2_TWIDDLE(u[0],u[8],mul_p0q8);	
	DFT2_TWIDDLE(u[1],u[9],mul_p1q8);
	DFT2_TWIDDLE(u[2],u[10],mul_p2q8);
	DFT2_TWIDDLE(u[3],u[11],mul_p3q8);
	DFT2_TWIDDLE(u[4],u[12],mul_p4q8);
	DFT2_TWIDDLE(u[5],u[13],mul_p5q8);
	DFT2_TWIDDLE(u[6],u[14],mul_p6q8);
	DFT2_TWIDDLE(u[7],u[15],mul_p7q8);

	DFT2_TWIDDLE(u[0],u[4],mul_p0q4);
  	DFT2_TWIDDLE(u[1],u[5],mul_p1q4);
	DFT2_TWIDDLE(u[2],u[6],mul_p2q4);
 	DFT2_TWIDDLE(u[3],u[7],mul_p3q4);
  	DFT2_TWIDDLE(u[8],u[12],mul_p0q4);
  	DFT2_TWIDDLE(u[9],u[13],mul_p1q4);
  	DFT2_TWIDDLE(u[10],u[14],mul_p2q4);
  	DFT2_TWIDDLE(u[11],u[15],mul_p3q4);

  	DFT2_TWIDDLE(u[0],u[2],mul_p0q2);
  	DFT2_TWIDDLE(u[1],u[3],mul_p1q2);
  	DFT2_TWIDDLE(u[4],u[6],mul_p0q2);
  	DFT2_TWIDDLE(u[5],u[7],mul_p1q2);
  	DFT2_TWIDDLE(u[8],u[10],mul_p0q2);
  	DFT2_TWIDDLE(u[9],u[11],mul_p1q2);
  	DFT2_TWIDDLE(u[12],u[14],mul_p0q2);
  	DFT2_TWIDDLE(u[13],u[15],mul_p1q2);

	float tmp = 1.0f / 16.0f;

	write_imagef (out, (int2) (x + 0, y), tmp *
		      (float4) (u[0] + u[1], 0, 0));
	write_imagef (out, (int2) (x + 1, y), tmp *
		      (float4) (u[8] + u[9], 0, 0));
	write_imagef (out, (int2) (x + 2, y), tmp *
		      (float4) (u[4] + u[5], 0, 0));
	write_imagef (out, (int2) (x + 3, y), tmp *
		      (float4) (u[12] + u[13], 0, 0));
	write_imagef (out, (int2) (x + 4, y), tmp *
		      (float4) (u[2] + u[3], 0, 0));
	write_imagef (out, (int2) (x + 5, y), tmp *
		      (float4) (u[10] + u[11], 0, 0));
	write_imagef (out, (int2) (x + 6, y), tmp *
		      (float4) (u[6] + u[7], 0, 0));
	write_imagef (out, (int2) (x + 7, y), tmp *
		      (float4) (u[14] + u[15], 0, 0));
	write_imagef (out, (int2) (x + 8, y), tmp *
		      (float4) (u[0] - u[1], 0, 0));
        write_imagef (out, (int2) (x + 9, y), tmp *
		      (float4) (u[8] - u[9], 0, 0));
	write_imagef (out, (int2) (x + 10, y), tmp *
		      (float4) (u[4] - u[5], 0, 0));
	write_imagef (out, (int2) (x + 11, y), tmp *
		      (float4) (u[12] - u[13], 0, 0));
	write_imagef (out, (int2) (x + 12, y), tmp *
		      (float4) (u[2] - u[3], 0, 0));
	write_imagef (out, (int2) (x + 13, y), tmp *
		      (float4) (u[10] + u[11], 0, 0));
	write_imagef (out, (int2) (x + 14, y), tmp *
		      (float4) (u[6] - u[7], 0, 0));
	write_imagef (out, (int2) (x + 15, y), tmp *
		      (float4) (u[14] - u[15], 0, 0));
}
