/*  
 * This file is part of Pentachoron.
 *
 * Pentachoron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pentachoron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Pentachoron.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "gbuffer.h"
#include "renderer.h"

GBuffer::GBuffer ()
#ifdef DEBUG
	: numsamples (0)
#endif
{
}

GBuffer::~GBuffer (void)
{
}

bool GBuffer::Init (void)
{
	if (!LoadProgram (quadtessprog, MakePath ("shaders", "bin",
																					 "gbuffer_quadtess.bin"),
										{ {"#version 420 core\n#define NUM_VERTICES 20\n"} },
										{ std::make_pair (GL_TESS_CONTROL_SHADER,
																			MakePath ("shaders", "gbuffer",
																								"tess", "control.txt")),
												 std::make_pair (GL_TESS_EVALUATION_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "tess", "quadeval.txt")),
												 std::make_pair (GL_VERTEX_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "tess", "vshader.txt")),
												 std::make_pair (GL_FRAGMENT_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "tess", "fshader.txt")) }))
		 return false;
	if (!LoadProgram (triangletessprog, MakePath ("shaders", "bin",
																					 "gbuffer_triangletess.bin"),
										{ {"#version 420 core\n#define NUM_VERTICES 15\n"} },
										{ std::make_pair (GL_TESS_CONTROL_SHADER,
																			MakePath ("shaders", "gbuffer",
																								"tess", "control.txt")),
												 std::make_pair (GL_TESS_EVALUATION_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "tess", "triangleeval.txt")),
												 std::make_pair (GL_VERTEX_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "tess", "vshader.txt")),
												 std::make_pair (GL_FRAGMENT_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "tess", "fshader.txt")) }))
		 return false;
	if (!LoadProgram (program, MakePath ("shaders", "bin", "gbuffer.bin"),
										{}, {	std::make_pair (GL_VERTEX_SHADER,
																					MakePath ("shaders", "gbuffer",
																										"vshader.txt")),
												 std::make_pair (GL_FRAGMENT_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "fshader.txt")) }))
		 return false;
	if (!LoadProgram (sraaprog, MakePath ("shaders", "bin", "gbuffer_sraa.bin"),
										{}, {	std::make_pair (GL_VERTEX_SHADER,
																					MakePath ("shaders", "gbuffer",
																										"vshader.txt")),
												 std::make_pair (GL_FRAGMENT_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "sraa.txt")) }))
		 return false;
	if (!LoadProgram (transparencyprog,
										MakePath ("shaders", "bin", "gbuffer_transparency.bin"),
										{}, {	std::make_pair (GL_VERTEX_SHADER,
																					MakePath ("shaders", "gbuffer",
																										"vshader.txt")),
												 std::make_pair (GL_FRAGMENT_SHADER,
																				 MakePath ("shaders", "gbuffer",
																									 "transparency.txt")) }))
		 return false;

	width = config["gbuffer"]["width"].as<GLuint> ();
	height = config["gbuffer"]["height"].as<GLuint> ();
	width = (width + 31) & (~0x1F);
	height = (height + 31) & (~0x1F);

	gl::Buffer::Unbind (GL_PIXEL_UNPACK_BUFFER);

	depthbuffer.Image2D (GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height,
												0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	colorbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
											 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	normalbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
												0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	specularbuffer.Image2D (GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
													0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
#ifdef DEBUG
	r->memory += width * height * 4 * 4;
#endif

	framebuffer.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
												 colorbuffer, 0);
	framebuffer.Texture2D (GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
												 normalbuffer, 0);
	framebuffer.Texture2D (GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
												 specularbuffer, 0);
	framebuffer.Texture2D (GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
												 depthbuffer, 0);
		
	framebuffer.DrawBuffers ({ GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
				 GL_COLOR_ATTACHMENT2 });

	fragidx.Image2D (GL_TEXTURE_2D, 0, GL_R32I, width, height,
									 0, GL_RED_INTEGER, GL_INT, NULL);
	// TODO: make this size configurable
	fraglist.Data (width * height * 4 * 4 * 8 , NULL, GL_DYNAMIC_COPY);

#ifdef DEBUG
	r->memory += width * height * (4 * 4 * 8 + 4);
#endif

	transparencyfb.Texture2D (GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
												 depthbuffer, 0);
	transparencyfb.DrawBuffers ({ });

	transparencyclearfb.Texture2D (GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
																 fragidx, 0);
	transparencyclearfb.DrawBuffers ({ GL_COLOR_ATTACHMENT0 });

	counter.Data (sizeof (GLuint) * 64, NULL, GL_DYNAMIC_DRAW);
	counter.ClearData (GL_R32UI, GL_RED, GL_UNSIGNED_INT, NULL);

	program["viewport"] = glm::uvec2 (width, height);
	program["farClipPlane"] = r->camera.GetFarClipPlane ();
	program["nearClipPlane"] = r->camera.GetNearClipPlane ();
	quadtessprog["viewport"] = glm::uvec2 (width, height);
	quadtessprog["farClipPlane"] = r->camera.GetFarClipPlane ();
	quadtessprog["nearClipPlane"] = r->camera.GetNearClipPlane ();
	triangletessprog["viewport"] = glm::uvec2 (width, height);
	triangletessprog["farClipPlane"] = r->camera.GetFarClipPlane ();
	triangletessprog["nearClipPlane"] = r->camera.GetNearClipPlane ();
	transparencyprog["viewport"] = glm::uvec2 (width, height);
	transparencyprog["farClipPlane"] = r->camera.GetFarClipPlane ();
	transparencyprog["nearClipPlane"] = r->camera.GetNearClipPlane ();
	sraaprog["viewport"] = glm::uvec2 (width, height);
	sraaprog["farClipPlane"] = r->camera.GetFarClipPlane ();
	sraaprog["nearClipPlane"] = r->camera.GetNearClipPlane ();

	wireframe = false;

	GL_CHECK_ERROR;

	return true;
}

void GBuffer::SetWireframe (bool w)
{
	wireframe = w;
}

bool GBuffer::GetWireframe (void)
{
	return wireframe;
}

void GBuffer::SetAntialiasing (GLuint samples)
{
#ifdef DEBUG
	r->memory -= width * height * numsamples * 4;
#endif
	if (samples > 1)
	{
		numsamples = samples;
		msdepthtexture.Image2DMultisample (GL_TEXTURE_2D_MULTISAMPLE,
																			 numsamples, GL_DEPTH_COMPONENT32,
																			 width, height, GL_TRUE);
		multisamplefb.Texture2D (GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D_MULTISAMPLE,
														 msdepthtexture, 0);
		multisamplefb.DrawBuffers ({ });

#ifdef DEBUG
		r->memory += width * height * numsamples * 4;
#endif
	}
	else
	{
		numsamples = 0;
		msdepthtexture = gl::Texture ();
		multisamplefb = gl::Framebuffer ();
	}
}

GLuint GBuffer::GetWidth (void)
{
	return width;
}

GLuint GBuffer::GetHeight (void)
{
	return height;
}

void GBuffer::SetProjMatrix (const glm::mat4 &projmat)
{
	program["projmat"] = projmat;
	quadtessprog["projmat"] = projmat;	
	triangletessprog["projmat"] = projmat;	
	transparencyprog["projmat"] = projmat;
	sraaprog["projmat"] = projmat;	
}

void GBuffer::Render (Geometry &geometry)
{
	r->culling.SetProjMatrix (r->camera.GetProjMatrix ());

	if (wireframe)
		 gl::PolygonMode (GL_FRONT_AND_BACK, GL_LINE);

	gl::Enable (GL_DEPTH_TEST);
	gl::DepthMask (GL_TRUE);
	gl::DepthFunc (GL_LESS);

	program.Use ();

	framebuffer.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, width, height);
		
	gl::ClearBufferfv (GL_COLOR, 0, (const float[]) {0.0f, 0.0f, 0.0f, 1.0f} );
	gl::ClearBufferfv (GL_COLOR, 1, (const float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_COLOR, 2, (const float[]) {0.0f, 0.0f, 0.0f, 0.0f} );
	gl::ClearBufferfv (GL_DEPTH, 0, (const float[]) {1.0f});

	geometry.Render (Geometry::Pass::GBuffer,
									 program, r->camera.GetViewMatrix ());

	quadtessprog.Use ();
	geometry.Render (Geometry::Pass::GBufferQuadTess,
									 quadtessprog, r->camera.GetViewMatrix ());

	triangletessprog.Use ();
	geometry.Render (Geometry::Pass::GBufferTriangleTess,
									 triangletessprog, r->camera.GetViewMatrix ());

	transparencyprog.Use ();

	gl::DepthMask (GL_FALSE);

	transparencyclearfb.Bind (GL_FRAMEBUFFER);

	gl::ClearBufferuiv (GL_COLOR, 0, (const GLuint[]) { (GLuint) -1, (GLuint) -1,
				 (GLuint) -1, (GLuint) -1 });
	
	transparencyfb.Bind (GL_FRAMEBUFFER);
	gl::Viewport (0, 0, width, height);

	fraglist.BindBase (GL_SHADER_STORAGE_BUFFER, 4);
	fragidx.BindImage (0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);
	counter.BindBase (GL_ATOMIC_COUNTER_BUFFER, 0);
	geometry.Render (Geometry::Pass::GBufferTransparency,
									 transparencyprog, r->camera.GetViewMatrix ());

	counter.ClearData (GL_R32UI, GL_RED, GL_UNSIGNED_INT, NULL);

	sraaprog.Use ();

	if (r->GetAntialiasing ())
	{
		gl::DepthMask (GL_TRUE);
		multisamplefb.Bind (GL_FRAMEBUFFER);
		gl::Viewport (0, 0, width, height);
		gl::ClearBufferfv (GL_DEPTH, 0, (const float[]) {1.0f});
			
		geometry.Render (Geometry::Pass::GBufferSRAA,
										 sraaprog, r->camera.GetViewMatrix ());
		gl::DepthMask (GL_FALSE);
	}

	gl::Framebuffer::Unbind (GL_FRAMEBUFFER);
	gl::Program::UseNone ();

	if (wireframe)
		 gl::PolygonMode (GL_FRONT_AND_BACK, GL_FILL);
	
	GL_CHECK_ERROR;
}
